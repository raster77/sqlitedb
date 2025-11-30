#ifndef SQLITEDB_HPP_
#define SQLITEDB_HPP_

#include <SqliteException.hpp>
#include <SqliteStatement.hpp>
#include <SqliteTypes.hpp>
#include <SqliteValueBinder.hpp>
#include <filesystem>
#include <mutex>
#include <ranges>
#include <string>

namespace sdb {

  class SqliteTransaction;

  class SqliteDb {
    public:
      /**
       * @brief Open a new database instance from a file.
       * @param filename Path to the SQLite database file.  If the file
       *   does not exist it will be created.
       * @return Unique pointer to a `SqliteDb` or `nullptr` on failure.
       */
      static std::unique_ptr<SqliteDb> open(const std::filesystem::path& filename, OpenMode mode = OpenMode::READ_WRITE);

      /**
       * @brief Create an in‑memory database.
       * @return Unique pointer to a new in‑memory `SqliteDb`.
       */
      static std::unique_ptr<SqliteDb> createInMemory();

      SqliteDb(const SqliteDb&) = delete;
      SqliteDb& operator=(const SqliteDb&) = delete;
      ~SqliteDb() = default;

      /**
       * @brief Check if the database connection is open.
       * @return true if open, false otherwise.
       */
      bool isOpen() const noexcept;

      /**
       * @brief Load a SQLite extension.
       * @param libraryPath Path to the shared library.
       * @param entryPoint The entry point function name.  Empty string
       *   chooses the default `sqlite3_xxx`.
       */
      void loadExtension(const std::filesystem::path& libraryPath, const std::string& entryPoint = "");

      /**
       * @brief Retrieve the last error message.
       * @return The SQLite error message string.
       */
      std::string getErrorMessage() const ;

      /**
       * @brief Retrieve the last error code.
       * @return The SQLite error code.
       */
      int getErrorCode() const;

      /**
       * @brief Return the row id of the last INSERT.
       * @return Last inserted row id.
       */
      std::int64_t getLastInsertedRowId();

      /**
       * @brief Query the SQLite library version.
       * @return Version string, e.g. "3.45.1".
       */
      std::string getVersion() const;

      /**
       * @brief Prepare a SQL statement.
       * @param sql SQL query.
       * @return A `SqliteStatement` object ready for execution.
       */
      SqliteStatement prepare(const std::string& sql);

      /**
       * @brief Execute a SQL statement that does not return rows.
       * @param sql SQL command.
       */
      void execute(const std::string& sql);

      /**
       * @brief Execute the same SQL statement repeatedly with a batch of parameter sets.
       *
       * The function prepares the supplied SQL once and then binds/executes it for
       * every row contained in @p values.  This is noticeably faster than calling
       * execute() or prepare()/step()/finalize() in a loop because:
       * - the statement is compiled only once
       * - the statement is reset (instead of being finalized) between rows
       *
       * @note  No transaction boundary is created.  If you need atomicity or
       *        periodic commits you must wrap the call yourself:
       * For very large batches you can commit every N rows by splitting the vector
       * or by calling execute("COMMIT") / execute("BEGIN") between chunks.
       *
       * @param sql    SQL text containing placeholders (`?`, `?NNN`, `:name`, `@name`
       *               or `$name`).  Must be the same for every row.
       * @param values A vector where each inner vector contains the parameter values
       *               for one execution. The size of the inner vector must match
       *               the number of placeholders in @p sql. Values are bound in the
       *               order they appear.
       *
       * @throw SqliteDbException if the database is not open, the SQL cannot be
       *        prepared, or a binding/step error occurs.  In case of an exception
       *        the statement that failed is the last one; earlier rows may have been
       *        executed already, so the caller must decide whether to rollback the
       *        surrounding transaction.
       */
      template<std::ranges::input_range Range>
        void executeBatch(const std::string& sql, Range&& rows);

      /**
       * @brief Optimize the database by running the VACUUM command.
       */
      void optimize();

      /**
       * @brief Enable or disable foreign key constraint enforcement.
       * @param value true to enable, false to disable.
       */
      void setForeignKeyOn(bool value);

      /**
       * @brief Set the journal mode.
       * @param mode The desired `JournalMode`.
       */
      void setJournalMode(JournalMode mode);

      /**
       * @brief Set the synchronous behaviour.
       * @param sync The desired `Synchronous` mode.
       */
      void setSynchronous(Synchronous sync);

      /**
       * @brief Set the temp store location.
       * @param store Desired `TempStore` type.
       */
      void setTempStore(TempStore store);

      /**
       * @brief Set the page cache size in kilobytes.
       * @param sizeKb Cache size in kilobytes.
       */
      void setCacheSize(std::size_t sizeKb);

    private:
      SqliteConnectionPtr mConnection;
      std::mutex mMutex;

      explicit SqliteDb(SqliteConnectionPtr connection);
      void checkConnection() const;

      friend class SqliteTransaction;
  };

  template<std::ranges::input_range Range>
  void SqliteDb::executeBatch(const std::string& sql, Range&& rows) {
      static_assert(std::ranges::input_range<Range>,
          "executeBatch requires an input range");

      if (!mConnection) {
        throw SqliteDbException("Database not open");
      }

      auto stmt = prepare(sql);
      SqliteValueBinder binder(stmt);

      for (auto&& row : std::forward<Range>(rows)) {
        std::size_t idx = 1;
        for (const auto& v : row) {
          binder.bind(static_cast<int>(idx++), v);
        }
        stmt.step();
        stmt.reset();
        stmt.clearBindings();
      }
  }


} /* namespace sql */

#endif /* SQLITEDB_HPP_ */
