#include "../sqlite/sqlite3.h"
#include <SqliteStatement.hpp>
#include <SqliteException.hpp>
#include <type_traits>
#include <variant>

namespace sdb {

  SqliteStatement::SqliteStatement(SqliteStatementPtr stmt)
      : mStatement(std::move(stmt)) {
  }

  void SqliteStatement::bind(int index, int value) {
    checkParameterIndex(index);
    int result = sqlite3_bind_int(mStatement.get(), index, value);
    if (result != SQLITE_OK) {
      throw SqliteStatementException("Failed to bind int at index " + std::to_string(index));
    }
  }

  void SqliteStatement::bind(int index, std::int64_t value) {
    checkParameterIndex(index);
    int result = sqlite3_bind_int64(mStatement.get(), index, value);
    if (result != SQLITE_OK) {
      throw SqliteStatementException("Failed to bind int64 at index " + std::to_string(index));
    }
  }

  void SqliteStatement::bind(int index, double value) {
    checkParameterIndex(index);
    int result = sqlite3_bind_double(mStatement.get(), index, value);
    if (result != SQLITE_OK) {
      throw SqliteStatementException("Failed to bind double at index " + std::to_string(index));
    }
  }

  void SqliteStatement::bind(int index, const std::string& value) {
    checkParameterIndex(index);
    int result = sqlite3_bind_text(mStatement.get(), index, value.c_str(), -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
      throw SqliteStatementException("Failed to bind text at index " + std::to_string(index));
    }
  }

  void SqliteStatement::bind(int index, const char* value) {
    checkParameterIndex(index);
    int result = sqlite3_bind_text(mStatement.get(), index, value, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
      throw SqliteStatementException("Failed to bind text at index " + std::to_string(index));
    }
  }

  void SqliteStatement::bind(int index, const std::vector<std::byte>& blob) {
    checkParameterIndex(index);
    int result = sqlite3_bind_blob(mStatement.get(), index, blob.data(),
                                   static_cast<int>(blob.size()), SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
      throw SqliteStatementException("Failed to bind text at index " + std::to_string(index));
    }
  }

  void SqliteStatement::bindNull(int index) {
    checkParameterIndex(index);
    int result = sqlite3_bind_null(mStatement.get(), index);
    if (result != SQLITE_OK) {
      throw SqliteStatementException("Failed to bind null at index " + std::to_string(index));
    }
  }

  int SqliteStatement::getInt(int column) const {
    checkColumnIndex(column);
    return sqlite3_column_int(mStatement.get(), column);
  }

  int64_t SqliteStatement::getInt64(int column) const {
    checkColumnIndex(column);
    return sqlite3_column_int64(mStatement.get(), column);
  }

  double SqliteStatement::getDouble(int column) const {
    checkColumnIndex(column);
    return sqlite3_column_double(mStatement.get(), column);
  }

  std::string SqliteStatement::getString(int column) const {
    checkColumnIndex(column);
    const unsigned char* text = sqlite3_column_text(mStatement.get(), column);
    if (text) {
      return reinterpret_cast<const char*>(text);
    }
    return "";
  }

  std::vector<std::byte> SqliteStatement::getBlob(int column) const {
    checkColumnIndex(column);
    int size = sqlite3_column_bytes(mStatement.get(), column);
    const std::byte* data = reinterpret_cast<const std::byte*>(sqlite3_column_blob(mStatement.get(), column));
    return std::vector<std::byte>(data, data + size);
  }

  bool SqliteStatement::isNull(int column) const {
    checkColumnIndex(column);
    return sqlite3_column_type(mStatement.get(), column) == SQLITE_NULL;
  }

  int SqliteStatement::getInt(const std::string& columnName) const {
    return getInt(getColumnIndex(columnName));
  }

  std::int64_t SqliteStatement::getInt64(const std::string& columnName) const {
    return getInt64(getColumnIndex(columnName));
  }

  double SqliteStatement::getDouble(const std::string& columnName) const {
    return getDouble(getColumnIndex(columnName));
  }

  std::string SqliteStatement::getString(const std::string& columnName) const {
    return getString(getColumnIndex(columnName));
  }

  std::vector<std::byte> SqliteStatement::getBlob(const std::string& columnName) const {
    return getBlob(getColumnIndex(columnName));
  }

  bool SqliteStatement::isNull(const std::string& columnName) const {
    return isNull(getColumnIndex(columnName));
  }

  void SqliteStatement::checkColumnIndex(int column) const {
    int columnCount = getColumnCount();
    if (column < 0 || column >= columnCount) {
      throw SqliteStatementException(
          "Column index " + std::to_string(column) + " out of range [0, " + std::to_string(columnCount - 1) + "]");
    }
  }

  std::string SqliteStatement::getSql() const {
      return sqlite3_sql(mStatement.get());
  }

  int SqliteStatement::getParameterCount() const {
    return sqlite3_bind_parameter_count(mStatement.get());
  }

  int SqliteStatement::getColumnCount() const {
    return sqlite3_column_count(mStatement.get());
  }

  std::string SqliteStatement::getColumnName(int column) const {
    checkColumnIndex(column);
    const char* name = sqlite3_column_name(mStatement.get(), column);
    return name ? name : "";
  }

  std::optional<int> SqliteStatement::getOptionalInt(int column) const {
    return isNull(column) ? std::optional<int> { } : std::optional { getInt(column) };
  }

  std::optional<int64_t> SqliteStatement::getOptionalInt64(int column) const {
    return isNull(column) ? std::optional<int64_t> { } : std::optional { getInt64(column) };
  }

  std::optional<double> SqliteStatement::getOptionalDouble(int column) const {
    return isNull(column) ? std::optional<double> { } : std::optional { getDouble(column) };
  }

  std::optional<std::string> SqliteStatement::getOptionalString(int column) const {
    return isNull(column) ? std::optional<std::string> { } : std::optional { getString(column) };
  }

  bool SqliteStatement::step() {
    int result = sqlite3_step(mStatement.get());

    if (result == SQLITE_ROW) {
      return true;
    } else if (result == SQLITE_DONE) {
      return false;
    } else {
      throw SqliteStatementException("Step failed with error code: " + std::to_string(result));
    }
  }

  void SqliteStatement::reset() {
    sqlite3_reset(mStatement.get());
  }

  void SqliteStatement::clearBindings() {
    sqlite3_clear_bindings(mStatement.get());
  }

  void SqliteStatement::checkParameterIndex(int index) const {
    if (index < 1) {
      throw SqliteStatementException("Parameter index must be >= 1, got " + std::to_string(index));
    }

    int paramCount = getParameterCount();
    if (index > paramCount) {
      throw SqliteStatementException(
          "Parameter index " + std::to_string(index) + " out of range [1, " + std::to_string(paramCount) + "]");
    }
  }

  int SqliteStatement::getColumnIndex(const std::string& name) const {
    int count = getColumnCount();
    for (int i = 0; i < count; ++i) {
      if (getColumnName(i) == name) {
        return i;
      }
    }

    throw SqliteStatementException("Column not found: " + name);
  }

} /* namespace sql */
