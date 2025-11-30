#include <SqliteValueBinder.hpp>

namespace sdb {

  SqliteValueBinder::SqliteValueBinder(SqliteStatement& stmt)
    : mStmt(stmt) {}

  void SqliteValueBinder::SqliteValueBinder::bind(int index, const SqliteValue& value) {
    std::visit([this, index](const auto& v) {
      using T = std::decay_t<decltype(v)>;
      if constexpr (std::is_same_v<T, std::monostate>) {
        mStmt.bindNull(index);
      } else if constexpr (std::is_same_v<T, std::int64_t>) {
        mStmt.bind(index, v);
      } else if constexpr (std::is_same_v<T, double>) {
        mStmt.bind(index, v);
      } else if constexpr (std::is_same_v<T, std::string>) {
        mStmt.bind(index, v);
      } else if constexpr (std::is_same_v<T, std::vector<std::byte>>) {
        mStmt.bind(index, v);
      }
    }, value);
  }
} /* namespace sdb */
