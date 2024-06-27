#include "sql.h"
#include <iostream>

auto SqlDatabase::query(std::string const& sql_query)
    -> std::variant<std::string, std::vector<std::vector<sql_value_t>>>
{
    if (!db_valid) {
        return error;
    }
    sqlite3_stmt* stmt{};
    int status =
        sqlite3_prepare(db, sql_query.c_str(), sql_query.size(), &stmt, 0);
    if (status != SQLITE_OK) {
        return "sqlite3_prepare() error: " + std::string(sqlite3_errmsg(db));
    }
    sql_response_t sql_response{};
    while (true) {
        std::vector<sql_value_t> sql_row{};
        status = sqlite3_step(stmt);
        if (status == SQLITE_BUSY) {
        } else if (status == SQLITE_DONE) {
            break;
        } else if (status == SQLITE_ROW) {
            int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                switch ((SqliteTypes)sqlite3_column_type(stmt, i)) {
                case SqliteTypes::Int:
                    sql_row.push_back(sqlite3_column_int(stmt, i));
                    break;
                case SqliteTypes::Float:
                    sql_row.push_back(sqlite3_column_double(stmt, i));
                    break;
                case SqliteTypes::Text: {
                    std::string str{
                        (const char*)sqlite3_column_text(stmt, i),
                        (size_t)sqlite3_column_bytes(stmt, i)
                    };
                    sql_row.push_back(str);
                } break;
                case SqliteTypes::Blob:
                    return "BLOBS are not implemented";
                    break;
                case SqliteTypes::Null:
                    sql_row.push_back({});
                    break;
                }
            }
        } else if (status == SQLITE_ERROR) {
            return "sqlite3_step() returned error";
            break;
        } else if (status == SQLITE_MISUSE) {
            return "sqlite3_step() returned misuse";
            break;
        }
        if (sql_row.size()) {
            sql_response.push_back(sql_row);
        }
    }
    status = sqlite3_finalize(stmt);
    if (status != SQLITE_OK) {
        return "sqlite3_finalize() returned error";
    }
    return sql_response;
}

SqlDatabase::SqlDatabase(std::string const& db_path)
{
    int status = sqlite3_open((db_path).c_str(), &db);
    if (status != SQLITE_OK) {
        error = "sqlite3_open() error: " + std::string(sqlite3_errmsg(db));
        sqlite3_free(db);
        db_valid = false;
    } else {
        db_valid = true;
    }
}

SqlDatabase::~SqlDatabase()
{
    if (db_valid) {
        int status = sqlite3_close(db);
        if (status != SQLITE_OK) {
            std::cerr << "sqlite3_close() error: " +
                             std::string(sqlite3_errmsg(db));
        }
    } else {
        sqlite3_free(db);
    }
}

auto operator<<(std::ostream& os, SqliteTypes t) -> std::ostream&
{
    switch (t) {
    case SqliteTypes::Int:
        os << "Int";
        break;
    case SqliteTypes::Float:
        os << "Float";
        break;
    case SqliteTypes::Text:
        os << "Text";
        break;
    case SqliteTypes::Blob:
        os << "Blob";
        break;
    case SqliteTypes::Null:
        os << "Null";
        break;
    }
    return os;
}

auto operator<<(std::ostream& os, std::vector<std::vector<sql_value_t>> res)
    -> std::ostream&
{
    for (auto const& row : res) {
        for (auto const& opt_col : row) {
            if (opt_col.has_value()) {
                auto const& val = opt_col.value();
                if (std::holds_alternative<int>(val)) {
                    os << get<int>(val);
                } else if (std::holds_alternative<double>(val)) {
                    os << get<double>(val);
                } else {
                    os << "\"" << get<std::string>(val) << "\"";
                }
            } else {
                os << "null";
            }
            os << " ";
        }
        os << "\n";
    }
    return os;
}
