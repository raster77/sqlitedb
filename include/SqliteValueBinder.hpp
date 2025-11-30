#ifndef INCLUDE_SQLITEVALUEBINDER_HPP_
#define INCLUDE_SQLITEVALUEBINDER_HPP_

#include <SqliteStatement.hpp>
#include <SqliteTypes.hpp>

namespace sdb {

  class SqliteValueBinder {
    public:
      explicit SqliteValueBinder(SqliteStatement& stmt);

      void bind(int index, const SqliteValue& value);

    private:
      SqliteStatement& mStmt;
  };

} /* namespace sdb */

#endif /* INCLUDE_SQLITEVALUEBINDER_HPP_ */
