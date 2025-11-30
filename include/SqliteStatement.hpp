#ifndef INCLUDE_SQLITESTATEMENT_HPP_
#define INCLUDE_SQLITESTATEMENT_HPP_

/**
 * @file SqliteStatement.hpp
 * @brief Wrapper around an SQLite prepared statement.
 * The class provides convenient binding helpers and value extraction
 * methods. All methods throw `SqliteException` on SQLite errors.
 */

#include <SqliteTypes.hpp>
#include <optional>
#include <vector>

namespace sdb {

/**
 * @brief Represents a compiled SQLite statement.
 */
class SqliteStatement {
public:
    /**
     * @brief Construct a statement from a raw pointer.
     * @param stmt SQLite statement pointer managed by the owning `SqliteDb`.
     */
    explicit SqliteStatement(SqliteStatementPtr stmt);

    /**
     * @brief Bind an integer to a named or positional parameter.
     * @param index 1‑based parameter index or name beginning with `@`.
     * @param value Integer value.
     */
    void bind(int index, int value);

    /**
     * @brief Bind a 64‑bit integer.
     * @param index 1‑based index.
     * @param value 64‑bit integer.
     */
    void bind(int index, std::int64_t value);

    /**
     * @brief Bind a double precision floating point value.
     * @param index 1‑based index.
     * @param value Double value.
     */
    void bind(int index, double value);

    /**
     * @brief Bind a string.
     * @param index 1‑based index.
     * @param value C++ string value.
     */
    void bind(int index, const std::string& value);

    /**
     *  @brief Bind a C‑string.
     *  @param index 1‑based index.
     *  @param value Null‑terminated string.
     */
    void bind(int index, const char* value);

    /**
     *  @brief Bind a binary blob.
     *  @param index 1‑based index.
     *  @param blob Vector of bytes.
     */
    void bind(int index, const std::vector<std::byte>& blob);

    /**
     *  @brief Bind a NULL value.
     *  @param index 1‑based index.
     */
    void bindNull(int index);

    /**
     *  @brief Bind a variadic list of arguments.
     *  @tparam Args Argument types.
     *  @param args Value pack to bind.
     */
    template<typename... Args>
    void bindAll(Args&&... args);

    /**
     *  @brief Retrieve an integer column by index.
     *  @param column 0‑based column index.
     *  @return Integer value.
     *
     */
    int getInt(int column) const;

    /**
     *  @brief Retrieve a 64‑bit integer column by index.
     *  @param column 0‑based index.
     *  @return 64‑bit integer.
     */
    std::int64_t getInt64(int column) const;

    /**
     *  @brief Retrieve a double column by index.
     *  @param column 0‑based index.
     *  @return Double value.
     */
    double getDouble(int column) const;

    /**
     *  @brief Retrieve a string column by index.
     *  @param column 0‑based index.
     *  @return String value.
     */
    std::string getString(int column) const;

    /**
     *  @brief Retrieve a binary blob column by index.
     *  @param column 0‑based index.
     *  @return Blob as byte vector.
     */
    std::vector<std::byte> getBlob(int column) const;

    /**
     *  @brief Check if a column is NULL by index.
     *  @param column 0‑based index.
     *  @return true if column is NULL.
     */
    bool isNull(int column) const;

    /**
     *  @brief Retrieve an integer by column name.
     *  @param columnName Column name.
     *  @return Integer value.
     */
    int getInt(const std::string& columnName) const;

    /**
     *  @brief Retrieve a 64‑bit integer by column name.
     *  @param columnName Column name.
     *  @return 64‑bit integer.
     */
    std::int64_t getInt64(const std::string& columnName) const;

    /**
     *  @brief Retrieve a double by column name.
     *  @param columnName Column name.
     *  @return Double value.
     */
    double getDouble(const std::string& columnName) const;

    /**
     *  @brief Retrieve a string by column name.
     *  @param columnName Column name.
     *  @return String value.
     */
    std::string getString(const std::string& columnName) const;

    /**
     *  @brief Retrieve a blob by column name.
     *  @param columnName Column name.
     *  @return Blob bytes.
     */
    std::vector<std::byte> getBlob(const std::string& columnName) const;

    /**
     *  @brief Check if a column is NULL by name.
     *  @param columnName Column name.
     *  @return true if NULL.
     */
    bool isNull(const std::string& columnName) const;

    /**
     *  @brief Get the SQL of the prepared statement.
     *  @return SQL string.
     */
    std::string getSql() const;

    /**
     *  @brief Number of parameters in the statement.
     *  @return Parameter count.
     */
    int getParameterCount() const;

    /**
     *  @brief Number of columns in the result set.
     *  @return Column count.
     */
    int getColumnCount() const;

    /**
     *  @brief Retrieve a column name.
     *  @param column 0‑based index.
     *  @return Column name.
     */
    std::string getColumnName(int column) const;

    /**
     *  @brief Retrieve an optional integer (may be null).
     *  @param column 0‑based index.
     *  @return Optional integer.
     */
    std::optional<int> getOptionalInt(int column) const;

    /**
     *  @brief Retrieve an optional 64‑bit integer.
     *  @param column 0‑based index.
     *  @return Optional 64‑bit integer.
     */
    std::optional<std::int64_t> getOptionalInt64(int column) const;

    /**
     *  @brief Retrieve an optional double.
     *  @param column 0‑based index.
     *  @return Optional double.
     */
    std::optional<double> getOptionalDouble(int column) const;

    /**
     *  @brief Retrieve an optional string.
     *  @param column 0‑based index.
     *  @return Optional string.
     */
    std::optional<std::string> getOptionalString(int column) const;

    /**
     *  @brief Advance to the next row.
     *  @return true if a row is available, false if done.
     */
    bool step();

    /**
     *  @brief Reset the statement to the initial state.
     */
    void reset();

    /**
     *  @brief Clear all bound parameters.
     */
    void clearBindings();

private:
    SqliteStatementPtr mStatement;

    template<typename T>
    void bindParameter(int index, T&& value);
    void checkColumnIndex(int column) const;
    void checkParameterIndex(int index) const;
    int getColumnIndex(const std::string& name) const;
};

template<typename... Args>
void SqliteStatement::bindAll(Args&&... args) {
    int index = 1;
    (bindParameter(index++, std::forward<Args>(args)), ...);
}

template<typename T>
void SqliteStatement::bindParameter(int index, T&& value) {
    if constexpr (std::is_same_v<std::decay_t<T>, int>) {
        bind(index, value);
    } else if constexpr (std::is_same_v<std::decay_t<T>, long long>) {
        bind(index, value);
    } else if constexpr (std::is_same_v<std::decay_t<T>, double>) {
        bind(index, value);
    } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        bind(index, value);
    } else if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
        bind(index, value);
    } else if constexpr (std::is_same_v<std::decay_t<T>, const std::vector<std::byte>&>) {
        bind(index, value);
    } else if constexpr (std::is_same_v<std::decay_t<T>, std::nullptr_t>) {
        bindNull(index);
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type for binding");
    }
}

} // namespace sdb

#endif /* INCLUDE_SQLITESTATEMENT_HPP_ */

