#include <SqliteTransaction.hpp>
#include <SqliteException.hpp>

namespace sdb {

  SqliteTransaction::SqliteTransaction(SqliteDb& sqliteDb, Mode mode)
    : mSqliteDb(sqliteDb)
    , mInTransaction(false) {
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
    mInTransaction = true;
  }

  SqliteTransaction::~SqliteTransaction() {
    if (mInTransaction) {
      try {
        mSqliteDb.execute("ROLLBACK");
        mInTransaction = false;
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
    return mInTransaction;
  }

  void SqliteTransaction::exec(const std::string& sql) {
    if (mInTransaction) {
      try {
        mSqliteDb.execute(sql);
        mInTransaction = false;
      } catch (SqliteException& e) {
        throw SqliteTransactionException(e.what(), e.errorCode());
      }
    }
  }

} /* namespace sql */
