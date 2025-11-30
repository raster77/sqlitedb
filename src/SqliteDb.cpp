#include "../sqlite/sqlite3.h"
#include <SqliteDb.hpp>
#include <SqliteStatement.hpp>
#include <format>

namespace sdb {

  SqliteDb::SqliteDb(SqliteConnectionPtr connection)
      : mConnection(std::move(connection)) {
  }

  bool SqliteDb::isOpen() const noexcept {
    return mConnection != nullptr;
  }

  std::unique_ptr<SqliteDb> SqliteDb::open(const std::filesystem::path& filename, OpenMode mode) {
    sqlite3* rawDb = nullptr;
    int flags = 0;

    switch (mode) {
      case OpenMode::READ_ONLY:
        flags = SQLITE_OPEN_READONLY;
        break;
      case OpenMode::READ_WRITE:
      default:
        flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        break;
    }

    int result = sqlite3_open_v2(filename.string().c_str(), &rawDb, flags, nullptr);

    if (result != SQLITE_OK) {
      throw SqliteDbException("Failed to open database: " + std::string(sqlite3_errmsg(rawDb)));
    }

    SqliteConnectionPtr connection(rawDb);
    auto db = std::unique_ptr<SqliteDb>(new SqliteDb(std::move(connection)));

    return db;
  }

  std::unique_ptr<SqliteDb> SqliteDb::createInMemory() {
    return open(":memory:", OpenMode::READ_WRITE);
  }

  void SqliteDb::loadExtension(const std::filesystem::path& libraryPath, const std::string& entryPoint) {
    if (!std::filesystem::exists(libraryPath)) {
      std::string error = "'" + libraryPath.string() + "' does not exist";
      throw SqliteDbException(error);
    }

    int result = sqlite3_enable_load_extension(mConnection.get(), 1);

    if (result != SQLITE_OK) {
      throw SqliteDbException("Failed to enable extension loading: " + getErrorMessage(), result);
    }

    auto disableExtensionGuard = [conn = mConnection.get()] {
      sqlite3_enable_load_extension(conn, 0);
    };

    char* errorMsg = nullptr;
    result = sqlite3_load_extension(mConnection.get(), libraryPath.string().c_str(),
                                    entryPoint.empty() ? nullptr : entryPoint.c_str(), &errorMsg);

    disableExtensionGuard();

    if (result != SQLITE_OK) {
      std::string error = "Failed to load extension '" + libraryPath.string() + "': ";
      if (errorMsg) {
        error += errorMsg;
        sqlite3_free(errorMsg);
      } else {
        error += sqlite3_errmsg(mConnection.get());
      }
      throw SqliteDbException(error, result);
    }
  }

  std::string SqliteDb::getErrorMessage() const {
    return mConnection ? sqlite3_errmsg(mConnection.get()) : "No database connection";
  }

  int SqliteDb::getErrorCode() const {
    return mConnection ? sqlite3_errcode(mConnection.get()) : -1;
  }

  std::int64_t SqliteDb::getLastInsertedRowId() {
    checkConnection();
    return static_cast<std::int64_t>(sqlite3_last_insert_rowid(mConnection.get()));
  }

  std::string SqliteDb::getVersion() const {
    return std::string(SQLITE_VERSION);
  }

  SqliteStatement SqliteDb::prepare(const std::string& sql) {
    checkConnection();

    std::lock_guard lock(mMutex);

    sqlite3_stmt* rawStmt = nullptr;
    int result = sqlite3_prepare_v2(mConnection.get(), sql.c_str(), -1, &rawStmt, nullptr);

    if (result != SQLITE_OK) {
      throw SqliteDbException("Failed to prepare statement: " + std::string(sqlite3_errmsg(mConnection.get())));
    }

    return SqliteStatement(SqliteStatementPtr(rawStmt));
  }

  void SqliteDb::execute(const std::string& sql) {
    checkConnection();

    std::lock_guard lock(mMutex);

    char* errMsg(nullptr);
    int rc = sqlite3_exec(mConnection.get(), sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
      std::string error;
      if (errMsg) {
        error = errMsg;
        sqlite3_free(errMsg);
      } else {
        error = getErrorMessage();
      }
      throw SqliteDbException("SQL execution error: " + error);
    }
  }

  void SqliteDb::optimize() {
    execute("VACUUM;");
  }

  void SqliteDb::setForeignKeyOn(bool value) {
    execute(std::format("PRAGMA foreign_keys = {}", (value ? "ON" : "OFF")));
  }

  void SqliteDb::setJournalMode(JournalMode mode) {
    static auto toString = [](JournalMode m) {
      switch (m) {
        case JournalMode::TRUNCATE:
          return "TRUNCATE";
        case JournalMode::PERSIST:
          return "PERSIST";
        case JournalMode::MEMORY:
          return "MEMORY";
        case JournalMode::WAL:
          return "WAL";
        case JournalMode::OFF:
          return "OFF";
        default:
          return "DELETE";
      }
    };

    execute(std::format("PRAGMA journal_mode = {}", toString(mode)));
  }

  void SqliteDb::setSynchronous(Synchronous sync) {
    static auto toString = [](Synchronous s) {
      switch (s) {
        case Synchronous::EXTRA:
          return "EXTRA";
        case Synchronous::NORMAL:
          return "NORMAL";
        case Synchronous::OFF:
          return "OFF";
        default:
          return "FULL";
      }
    };

    execute(std::format("PRAGMA synchronous = {}", toString(sync)));
  }

  void SqliteDb::setTempStore(TempStore store) {
    static auto toString = [](TempStore t) {
      switch (t) {
        case TempStore::FILE:
          return "FILE";
        case TempStore::MEMORY:
          return "MEMORY";
        default:
          return "DEFAULT";
      }
    };

    execute(std::format("PRAGMA temp_store = {}", toString(store)));
  }

  void SqliteDb::setCacheSize(std::size_t sizeKb) {
    constexpr std::size_t MIN_CACHE_KB = 0;
    constexpr std::size_t MAX_CACHE_KB = 2000000;

    if (sizeKb < MIN_CACHE_KB) {
      throw std::invalid_argument(
          "Cache size too small: " + std::to_string(sizeKb) + " KB (minimum: " + std::to_string(MIN_CACHE_KB)
              + " KB)");
    }

    if (sizeKb > MAX_CACHE_KB) {
      throw std::invalid_argument(
          "Cache size too large: " + std::to_string(sizeKb) + " KB (maximum: " + std::to_string(MAX_CACHE_KB)
              + " KB)");
    }

    execute(std::format("PRAGMA cache_size = {}", static_cast<std::int64_t>(sizeKb)));
  }

  void SqliteDb::checkConnection() const {
    if (!mConnection) {
      throw SqliteDbException("Database not open");
    }
  }

} /* namespace sql */
