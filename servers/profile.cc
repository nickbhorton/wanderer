#include <cctype>
#include <format>
#include <meadow_server.h>
#include <sqlite3.h>
#include <sstream>
#include <string>
#include <variant>

#include "sql.h"

// server funcs
auto not_found(TcpServer& server)
{
    server.write_serialized_string_connection("1");
    server.close_connection();
}

// text functions
auto split_on(std::string const& query, char split_char)
    -> std::vector<std::string>
{
    std::vector<std::string> result{};
    std::string processing{query};
    while (true) {
        auto pos = processing.find_first_of(split_char);
        if (pos == std::string::npos) {
            break;
        } else {
            std::string add{processing.substr(0, pos)};
            result.push_back(add);
            processing = processing.substr(pos + 1);
        }
    }
    if (processing.size()) {
        result.push_back(processing);
    }
    return result;
}

auto lower(std::string const& in) -> std::string
{
    std::string result{};
    for (auto const& c : in) {
        result.push_back(std::tolower(c));
    }
    return result;
}

// server funcs
auto process_query(std::string const& query)
    -> std::optional<std::tuple<std::string, std::string, int>>
{
    std::vector<std::string> fields{split_on(query, '&')};
    /*
    for (auto const& f : fields) {
        std::cout << f << "\n";
    }
    */
    if (fields.size() < 3) {
        return {};
    }
    std::string firstname{};
    std::string lastname{};
    int id{};
    {
        std::vector<std::string> args{split_on(fields[0], '=')};
        if (args.size() < 2) {
            return {};
        }
        firstname = lower(args[1]);
    }
    {
        std::vector<std::string> args{split_on(fields[1], '=')};
        if (args.size() < 2) {
            return {};
        }
        lastname = lower(args[1]);
    }
    {
        std::vector<std::string> args{split_on(fields[2], '=')};
        if (args.size() < 2) {
            return {};
        }
        id = std::stoi(args[1]);
    }
    return {{firstname, lastname, id}};
}

static auto mi_to_k(double mi) -> double { return mi * 1.60934; }
/*
CREATE TABLE ultras (
id INTEGER PRIMARY KEY,
profile_id INTEGER NOT NULL,
start_time DATETIME, yyyy-MM-dd hh:mm:ss
time TIME, hh:mm:ss
distance_mi REAL,
elevation_gain_ft REAL
);


        <div class="ultra m100">
            <div class="py-2 flex justify-evenly w-full">
                <div>
                    100.0
                </div>
                <div>
                    21:31:14
                </div>
                <div>
                    4531
                </div>
            </div>
        </div>
*/
auto process_ultra(std::vector<sql_value_t> const& ultra) -> std::string
{
    std::stringstream ss{};
    double distance_mi = std::get<double>(ultra[4].value());
    double elevation_gain_ft = std::get<double>(ultra[5].value());
    std::string time = std::get<std::string>(ultra[3].value());
    ss << "<div class=\"ultra ";

    if (mi_to_k(distance_mi) < 49) {
        ss << "marathon ";
    } else if (distance_mi < 49) {
        ss << "k50 ";
    } else if (mi_to_k(distance_mi) < 98) {
        ss << "m50 ";
    } else if (distance_mi < 99) {
        ss << "k100 ";
    } else {
        ss << "m100 ";
    }
    ss << "\">";

    ss << "<div class=\"py-2 flex justify-evenly w-full\">";

    ss << "<div>";
    ss << distance_mi;
    ss << "</div>";

    ss << "<div>";
    ss << time;
    ss << "</div>";

    ss << "<div>";
    ss << elevation_gain_ft;
    ss << "</div>";

    ss << "</div>";

    ss << "</div>";
    return ss.str();
}

auto process_ultras(sql_response_t const& data) -> std::string
{
    std::stringstream result{};
    result << "<!DOCTYPE html>";
    result << "<html lang=\"en\">";

    result << "<head>";
    result << "<meta charset=\"utf-8\">";
    result << "<meta name=\"viewport\" content=\"width=device-width, "
              "initial-scale=1\">";
    result << "<meta name=\"color-scheme\" content=\"light dark\"/>";
    result << "<script src=\"https://unpkg.com/htmx.org@2.0.0\"></script>";
    result << "<link href=\"output.css\"rel=\"stylesheet\">";
    result << "</head>";
    result << "<body>";
    result << "<div class=\"flex justify-center flex-wrap\">";
    for (auto const& ultra : data) {
        result << process_ultra(ultra);
    }
    result << "</div>";
    result << "</body>";
    return result.str();
}

std::string const db_path{"/home/nick/dev/wanderer/build/data/wanderer.db"};

int main()
{
    SqlDatabase db(db_path);
    TcpServer server("127.0.0.1", 50000);
    while (true) {
        server.find_connection(5);
        if (server.is_valid()) {
            std::string query{server.read_serialized_string_connection()};
            auto const& opt = process_query(query);
            if (!opt.has_value()) {
                not_found(server);
                continue;
            }
            auto const& [firstname, lastname, name_id] = opt.value();
            std::string const id_query{std::format(
                "SELECT id FROM profiles WHERE firstname='{}' AND "
                "lastname='{}' AND name_id='{}';",
                firstname,
                lastname,
                name_id
            )};
            std::cout << id_query << "\n";
            auto id_result{db.query(id_query)};
            if (std::holds_alternative<std::string>(id_result)) {
                std::stringstream const ss{std::get<std::string>(id_result)};
                server.write_serialized_string_connection(ss.str());
            } else {
                auto const res{std::get<sql_response_t>(id_result)};
                if (res.size() && res[0].size() && res[0][0].has_value() &&
                    std::holds_alternative<int>(res[0][0].value())) {

                    int const profile_id{std::get<int>(res[0][0].value())};
                    std::string const ultras_query{std::format(
                        "SELECT * FROM ultras WHERE profile_id='{}' ORDER BY "
                        "distance_mi DESC;",
                        profile_id
                    )};
                    std::cout << ultras_query << "\n";
                    auto ultras_result{db.query(ultras_query)};
                    if (std::holds_alternative<std::string>(ultras_result)) {
                        std::stringstream ss{};
                        ss << std::get<std::string>(ultras_result) << "\n";
                        server.write_serialized_string_connection(ss.str());
                    } else {
                        server.write_serialized_string_connection(
                            process_ultras(
                                std::get<sql_response_t>(ultras_result)
                            )
                        );
                    }
                } else {
                    not_found(server);
                }
            }
        }
    }
}
