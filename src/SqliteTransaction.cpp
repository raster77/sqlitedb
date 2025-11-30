#include <SqliteTransaction.hpp>
#include <SqliteException.hpp>

namespace sdb {

  SqliteTransaction::SqliteTransaction(SqliteDb& sqliteDb, Mode mode)
    : mSqliteDb(sqliteDb)
    , mIntransaction (false) {
    if (!sqliteDb.isOpen()) {
      throw SqliteTransactionException("No database");
    }

    const std::string sql = [mode] {
      switch (mode) {
        case Mode::Deferred:  return "BEGIN DEFERRED;";
        case Mode::Exclusive: return "BEGIN EXCLUSIVE;";
        default: return "BEGIN IMMEDIATE;";
      }
    }();

    sqliteDb.execute(sql);
    mIntransaction = true;
  }

  SqliteTransaction::~SqliteTransaction() {
    if (mIntransaction) {
      try {
        mSqliteDb.execute("ROLLBACK");
        mIntransaction = false;
      } catch (...) {
        // Log error silently
      }
    }
  }

  void SqliteTransaction::commit() {
    exec("COMMIT");
  }

  void SqliteTransaction::rollback() {
    exec("ROLLBACK");
  }

  bool SqliteTransaction::inTransaction() const {
    return mIntransaction;
  }

  void SqliteTransaction::exec(const std::string& sql) {
    if (mIntransaction) {
      try {
        mSqliteDb.execute(sql);
        mIntransaction = false;
      } catch (SqliteException& e) {
        throw SqliteTransactionException(e.what());
      }
    }
  }

} /* namespace sql */
