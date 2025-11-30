#ifndef INCLUDE_SQLITEEXCEPTION_HPP_
#define INCLUDE_SQLITEEXCEPTION_HPP_

#include <stdexcept>
#include <string>

namespace sdb {

  class SqliteException : public std::runtime_error {
    public:
      explicit SqliteException(const std::string& message, int error_code = 0);

      int errorCode() const noexcept {
        return mErrorCode;
      }

    private:
      int mErrorCode;
  };

  class SqliteDbException : public SqliteException {
    public:
      using SqliteException::SqliteException;
  };

  class SqliteStatementException : public SqliteException {
    public:
      using SqliteException::SqliteException;
  };

  class SqliteTransactionException : public SqliteException {
    public:
      using SqliteException::SqliteException;
  };

}

#endif /* INCLUDE_SQLITEEXCEPTION_HPP_ */
