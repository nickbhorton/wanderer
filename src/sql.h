#pragma once

#include <optional>
#include <sqlite3.h>
#include <string>
#include <variant>
#include <vector>

typedef std::optional<std::variant<int, double, std::string>> sqlite_result_t;

enum class SqliteTypes {
    Int = SQLITE_INTEGER,
    Float = SQLITE_FLOAT,
    Text = SQLITE_TEXT,
    Blob = SQLITE_BLOB,
    Null = SQLITE_NULL
};
auto operator<<(std::ostream& os, SqliteTypes t) -> std::ostream&;
auto operator<<(std::ostream& os, std::vector<std::vector<sqlite_result_t>> res)
    -> std::ostream&;

class SqlDatabase
{
    sqlite3* db;
    bool db_valid;
    std::string error;

public:
    SqlDatabase(std::string const& path);
    ~SqlDatabase();
    auto query(std::string const& sql_query)
        -> std::variant<std::string, std::vector<std::vector<sqlite_result_t>>>;
};
