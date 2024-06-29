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
CREATE TABLE profiles (
    id INTEGER PRIMARY KEY,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    firstname TEXT NOT NULL,
    lastname TEXT NOT NULL,
    name_id INTEGER NOT NULL,
    birth_day DATE NOT NULL
)
*/
auto get_profile_header(sql_response_t users_result) -> std::string
{
    std::stringstream ss{};
    std::string firstname{std::get<std::string>(users_result[0][2].value())};
    firstname[0] = toupper(firstname[0]);
    std::string lastname{std::get<std::string>(users_result[0][3].value())};
    lastname[0] = toupper(lastname[0]);
    int name_id{std::get<int>(users_result[0][4].value())};
    std::string birthday{std::get<std::string>(users_result[0][5].value())};
    ss << "<div class=\"profile_header\">";
    ss << "<div>";
    ss << firstname << " ";
    ss << lastname;
    ss << "</div>";
    ss << "<div>";
    ss << birthday;
    ss << "</div>";
    ss << "</div>";
    return ss.str();
}

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
auto get_ultra_summary_html(std::vector<sql_value_t> const& ultra)
    -> std::string
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

    ss << "<div class=\"py-2 flex justify-around w-full\">";

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

auto get_ultras_summary_html(sql_response_t const& data) -> std::string
{
    std::stringstream result{};
    result << "<div class=\"flex justify-center flex-wrap\">";
    for (auto const& ultra : data) {
        result << get_ultra_summary_html(ultra);
    }
    result << "</div>";
    return result.str();
}

auto send_profile(SqlDatabase& db, std::string const& url) -> std::string
{
    int const profile_id = std::stoi(url.substr(1));
    std::string const users_query{
        std::format("SELECT * FROM profiles WHERE id='{}'", profile_id)
    };
    std::cout << users_query << "\n";
    auto users_result{db.query(users_query)};
    if (std::holds_alternative<std::string>(users_result)) {
        std::stringstream ss{};
        ss << std::get<std::string>(users_result) << "\n";
        return ss.str();
    }
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
        return ss.str();
    }
    std::stringstream ss{};
    ss << "<div class=\"profile_holder\">";
    ss << get_profile_header(std::get<sql_response_t>(users_result));
    ss << get_ultras_summary_html(std::get<sql_response_t>(ultras_result));
    ss << "</div>";
    return ss.str();
}

/*
<div hx-get="/profile?firstname=nicholas&lastname=horton&number_id=1"
hx-trigger="click" hx-target="#content-target" class="flex p-4 cursor-pointer">
    <div>
        nicholas horton
    </div>
    <div class="text-slate-300 ml-1">
        #1
    </div>
</div>

CREATE TABLE profiles (
    id INTEGER PRIMARY KEY,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    firstname TEXT NOT NULL,
    lastname TEXT NOT NULL,
    name_id INTEGER NOT NULL,
    birth_day DATE NOT NULL
)

*/
auto get_profile_html(std::vector<sql_value_t> const& res) -> std::string
{
    std::stringstream ss{};
    std::string firstname{std::get<std::string>(res[0].value())};
    std::string lastname{std::get<std::string>(res[1].value())};
    int number_id{std::get<int>(res[2].value())};
    int id{std::get<int>(res[3].value())};
    ss << std::format(
        "<div "
        "hx-get=\"/profiles/{}\"",
        id
    );
    ss << "hx-trigger=\"click\" hx-target=\"#content-target\" class=\"flex p-4 "
          "cursor-pointer\">";

    ss << "<div>";
    ss << std::format("{} {}", firstname, lastname);
    ss << "</div>";
    ss << "<div class=\"profile_number_id\">";
    ss << std::format("#{}", number_id);
    ss << "</div>";

    ss << "</div>";
    return ss.str();
}

auto get_profiles_summary_html(SqlDatabase& db) -> std::string
{
    std::string const profiles_query{
        std::format("SELECT firstname, lastname, name_id, id FROM profiles;")
    };
    std::cout << profiles_query << "\n";
    auto const res_err = db.query(profiles_query);
    if (std::holds_alternative<std::string>(res_err)) {
        return std::get<std::string>(res_err);
    }
    auto const& res = std::get<sql_response_t>(res_err);
    std::stringstream ss{};
    for (auto const& row : res) {
        ss << get_profile_html(row);
    }
    return ss.str();
}

std::string const db_path{"/home/nick/dev/wanderer/build/data/wanderer.db"};

int main()
{
    SqlDatabase db(db_path);
    TcpServer server("127.0.0.1", 50000);
    while (true) {
        server.find_connection(5);
        if (server.is_valid()) {
            std::string method{server.read_serialized_string_connection()};
            std::cout << "method: " << method << "\n";
            std::string url{server.read_serialized_string_connection()};
            std::cout << "url: " << url << "\n";
            std::string query{server.read_serialized_string_connection()};
            std::string response{};
            if (url == "/") {
                response = get_profiles_summary_html(db);
            } else {
                response = send_profile(db, url);
            }
            if (response.size()) {
                server.write_serialized_string_connection(response);
            } else {
                server.write_serialized_string_connection("1");
            }
            server.close_connection();
        }
    }
}
