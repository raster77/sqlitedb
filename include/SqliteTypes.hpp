#ifndef INCLUDE_SQLITETYPES_HPP_
#define INCLUDE_SQLITETYPES_HPP_

#include "../sqlite/sqlite3.h"
#include <string>
#include <memory>
#include <vector>
#include <variant>

namespace sdb {

  enum class OpenMode {
    READ_WRITE, READ_ONLY
  };

  enum class JournalMode {
    DELETE, TRUNCATE, PERSIST, MEMORY, WAL, OFF
  };

  enum class Synchronous {
    OFF, NORMAL, FULL, EXTRA
  };

  enum class TempStore {
    DEFAULT, FILE, MEMORY
  };

  enum class SecureDelete {
    OFF, ON, FAST
  };

  struct SqliteConnectionDeleter {
    void operator()(sqlite3* db) const {
      if (db) {
        sqlite3_close_v2(db);
      }
    }
  };

  struct SqliteStatementDeleter {
    void operator()(sqlite3_stmt* stmt) const {
      if (stmt) {
        sqlite3_finalize(stmt);
      }
    }
  };

  using SqliteConnectionPtr = std::unique_ptr<sqlite3, SqliteConnectionDeleter>;
  using SqliteStatementPtr = std::unique_ptr<sqlite3_stmt, SqliteStatementDeleter>;

  using SqliteValue = std::variant<
    std::monostate,
    std::int64_t,
    double,
    std::string,
    std::vector<std::byte>
  >;

  inline SqliteValue sqliteNull() {
    return std::monostate { };
  }

  template<typename T>
    SqliteValue sqliteValue(T&& val) {
      return SqliteValue { std::forward<T>(val) };
    }

}

#endif /* INCLUDE_SQLITETYPES_HPP_ */
