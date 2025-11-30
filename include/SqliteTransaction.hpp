#ifndef INCLUDE_TRANSACTION_HPP_
#define INCLUDE_TRANSACTION_HPP_

#include "SqliteDb.hpp"

namespace sdb {

/**
 * @brief Lightweight RAII wrapper for SQLite transactions.
 *
 * This class begins a transaction on construction and
 * automatically rolls back if `commit()` is not called
 * before destruction. It is designed to work with the
 * `sdb::SqliteDb` interface.
 *
 * Example usage:
 * @code
 *   sdb::SqliteDb db("file:example.db?mode=rwc");
 *   sdb::SqliteTransaction trans(db);
 *   // ... perform queries
 *   trans.commit(); // Persist changes
 * @endcode
 */
class SqliteTransaction {
    public:

    enum class Mode { Deferred, Immediate, Exclusive };

    /**
   * @brief Construct a new SqliteTransaction.
   *
   * The constructor begins an SQLite transaction by
   * executing a `BEGIN` statement on the supplied database.
   * If the transaction cannot be started, an exception
   * of type `SqliteException` will be thrown.
   *
   * @param sqliteDb Reference to the database object.
   */
  SqliteTransaction(SqliteDb& sqliteDb, Mode mode = Mode::Immediate);
  /**
   * @brief Destroy the SqliteTransaction.
   *
   * If `commit()` has not been called, the transaction is
   * automatically rolled back. Any exception thrown from the
   * destructor will be suppressed to avoid terminating the
   * program during stack unwinding.
   */
  ~SqliteTransaction();

  /**
   * @brief Commit the current transaction.
   *
   * Executes a `COMMIT` statement on the database. After
   * a successful commit this object switches to a no‑op
   * state to prevent double commits or rollbacks.
   *
   * @throws sdb::SqliteDbException if the commit fails.
   */
  void commit();
  /**
   * @brief Rollback the current transaction.
   *
   * Executes a `ROLLBACK` statement on the database.
   * After a rollback the transaction is considered
   * finished and subsequent calls to `commit()` or
   * `rollback()` are no-ops.
   *
   * @throws sdb::SqliteDbException if the rollback fails.
   */
  void rollback();

  /**
   * @brief Checks whether a transaction is currently active.
   *
   * @return true if the database is currently inside a transaction,
   *         false otherwise.
   */
  bool inTransaction() const;

    private:
      SqliteDb& mSqliteDb;
      bool mIntransaction;
      void exec(const std::string& sql);
  };

} /* namespace sql */

#endif /* INCLUDE_TRANSACTION_HPP_ */
