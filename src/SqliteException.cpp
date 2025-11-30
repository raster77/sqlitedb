#include <SqliteException.hpp>

namespace sdb {

  SqliteException::SqliteException(const std::string& message, int error_code)
    : std::runtime_error(message)
    , mErrorCode(error_code) {
  }

}
