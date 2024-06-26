#include <cctype>
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

// html funcs
auto p(std::string const& in) -> std::string { return "<p>" + in + "</p>"; }
auto hr() -> std::string { return "<hr>"; }

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
            std::string sql_query =
                "SELECT id FROM profiles WHERE firstname='" + firstname +
                "' AND lastname='" + lastname + "' AND name_id='" +
                std::to_string(name_id) + "';";
            std::cout << sql_query << "\n";
            auto sql_result = db.query(sql_query);
            if (std::holds_alternative<std::string>(sql_result)) {
                std::stringstream ss{};
                ss << std::get<std::string>(sql_result) << "\n";
                server.write_serialized_string_connection(ss.str());
                continue;
            } else {
                auto vec = std::get<std::vector<std::vector<sqlite_result_t>>>(
                    sql_result
                );
                if (vec.size() && vec[0].size() && vec[0][0].has_value() &&
                    std::holds_alternative<int>(vec[0][0].value())) {
                    int profile_id = std::get<int>(vec[0][0].value());
                    std::string sql_query =
                        "SELECT * FROM ultras WHERE profile_id='" +
                        std::to_string(profile_id) + "';";
                    std::cout << sql_query << "\n";
                    auto sql_result = db.query(sql_query);
                    std::stringstream ss{};
                    if (std::holds_alternative<std::string>(sql_result)) {
                        ss << std::get<std::string>(sql_result) << "\n";
                        server.write_serialized_string_connection(ss.str());
                    } else {
                        ss << std::get<
                                  std::vector<std::vector<sqlite_result_t>>>(
                                  sql_result
                              )
                           << "\n";
                        server.write_serialized_string_connection(ss.str());
                    }
                } else {
                    server.write_serialized_string_connection("no ultras found"
                    );
                }
            }
        }
    }
}
