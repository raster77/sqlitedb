# sqlitedb

A modern C++20 RAII wrapper for SQLite3, providing type-safe database operations with exception handling.

## Features

- **RAII Resource Management**: Automatic cleanup of database connections and prepared statements
- **Type-Safe Interface**: Compile-time type checking for bindings and value extraction
- **Exception Handling**: Comprehensive error reporting with SQLite error codes
- **Modern C++20**: Uses contemporary C++ features including variants, ranges, and concepts
- **Transaction Support**: RAII transactions with automatic rollback
- **Batch Operations**: Efficient batch execution with range support
- **Flexible Binding**: Support for named and positional parameters
- **Memory Safety**: Smart pointers for automatic resource management

## Requirements

- C++20 compatible compiler
- SQLite3 library
- CMake 3.15+

## Project Structure

sqlitedb/  
├── include/ # Public header files  
├── sqlite/ # SQLite3 source/headers  
├── src/ # Implementation files  
└── CMakeLists.txt # Build configuration


## Quick Start

```cpp
#include <sqlitedb/SqliteDb.hpp>
#include <sqlitedb/SqliteTransaction.hpp>

using namespace sdb;

// Open a database
auto db = SqliteDb::open("example.db");

// Create a table
db->execute(R"(
    CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY,
        name TEXT NOT NULL,
        age INTEGER
    )
)");

// Insert data with transaction
{
    SqliteTransaction trans(*db);
    auto stmt = db->prepare("INSERT INTO users (name, age) VALUES (?, ?)");
    stmt.bindAll("John Doe", 30);
    stmt.step();
    trans.commit();
}

// Query data
auto stmt = db->prepare("SELECT id, name, age FROM users WHERE age > ?");
stmt.bind(1, 25);

while (stmt.step()) {
    auto id = stmt.getInt64(0);
    auto name = stmt.getString(1);
    auto age = stmt.getOptionalInt(2);  // Handles NULL values
    
    std::cout << "User: " << name << " (ID: " << id << ")\n";
}
```

## Key Components

### SqliteDb

Main database connection class:

    - Open database files or create in-memory databases
    - Prepare statements and execute SQL
    - Configure database settings (journal mode, foreign keys, etc.)
    - Batch operations for efficient bulk inserts

### SqliteStatement

Prepared statement wrapper:

    - Type-safe parameter binding
    - Flexible value extraction with optional types
    - Support for all SQLite data types (INTEGER, REAL, TEXT, BLOB, NULL)
    - Named and positional parameter access

### SqliteTransaction

RAII transaction management:

    - Automatic rollback on destruction if not committed
    - Support for deferred, immediate, and exclusive transactions
    - Exception-safe operation

### SqliteValue

Type-safe value representation using std::variant:

    - std::monostate for NULL values
    - std::int64_t for integers
    - double for floating-point numbers
    - std::string for text
    - std::vector<std::byte> for binary data

## Advanced Usage

### Batch Operations

```cpp
std::vector<std::vector<SqliteValue>> users = {
    {sqliteValue("Alice"), sqliteValue(25)},
    {sqliteValue("Bob"), sqliteValue(30)},
    {sqliteValue("Charlie"), sqliteNull()}
};

db->executeBatch(
    "INSERT INTO users (name, age) VALUES (?, ?)",
    users
);
```

### Transaction with Error Handling

```cpp
try {
    SqliteTransaction trans(*db, SqliteTransaction::Mode::Immediate);
    
    // Perform multiple operations
    db->execute("UPDATE accounts SET balance = balance - 100 WHERE id = 1");
    db->execute("UPDATE accounts SET balance = balance + 100 WHERE id = 2");
    
    trans.commit();
} catch (const SqliteException& e) {
    std::cerr << "Transaction failed: " << e.what() << std::endl;
    // Transaction automatically rolled back
}
```

### Custom Configuration

```cpp
auto db = SqliteDb::open("app.db");
db->setForeignKeyOn(true);
db->setJournalMode(JournalMode::WAL);
db->setSynchronous(Synchronous::NORMAL);
db->setCacheSize(2000); // 2MB cache
```

### Error Handling

All operations throw exceptions derived from SqliteException:

    - SqliteDbException: Database-level errors
    - SqliteStatementException: Statement preparation/execution errors
    - SqliteTransactionException: Transaction-related errors
	
```cpp
try {
    auto stmt = db->prepare("INVALID SQL");
} catch (const SqliteStatementException& e) {
    std::cout << "SQL error: " << e.what() 
              << " (code: " << e.errorCode() << ")" << std::endl;
}
```

## API Reference

### SqliteDb Class

#### Static Methods

```cpp
    static std::unique_ptr<SqliteDb> open(const std::filesystem::path& filename, OpenMode mode = OpenMode::READ_WRITE)
    static std::unique_ptr<SqliteDb> createInMemory()
```

#### Instance Methods

```cpp
    SqliteStatement prepare(const std::string& sql)
    void execute(const std::string& sql)
    template<std::ranges::input_range Range> void executeBatch(const std::string& sql, Range&& rows)
    bool isOpen() const noexcept
    std::int64_t getLastInsertedRowId()
    void setForeignKeyOn(bool value)
    void setJournalMode(JournalMode mode)
    void setSynchronous(Synchronous sync)
    void setTempStore(TempStore store)
    void setCacheSize(std::size_t sizeKb)
```

### SqliteStatement Class

#### Binding Methods

```cpp
    void bind(int index, int value)
    void bind(int index, std::int64_t value)
    void bind(int index, double value)
    void bind(int index, const std::string& value)
    void bind(int index, const char* value)
    void bind(int index, const std::vector<std::byte>& blob)
    void bindNull(int index)
    template<typename... Args> void bindAll(Args&&... args)
```

#### Value Extraction Methods

```cpp
    int getInt(int column) const
    std::int64_t getInt64(int column) const
    double getDouble(int column) const
    std::string getString(int column) const
    std::vector<std::byte> getBlob(int column) const
    bool isNull(int column) const
```

#### Optional Value Extraction

```cpp
    std::optional<int> getOptionalInt(int column) const
    std::optional<std::int64_t> getOptionalInt64(int column) const
    std::optional<double> getOptionalDouble(int column) const
    std::optional<std::string> getOptionalString(int column) const
```

#### Execution Methods

```cpp
    bool step() - Advance to next row, returns true if row available
    void reset() - Reset statement to initial state
    void clearBindings() - Clear all bound parameters
```

### SqliteTransaction Class

#### Modes

    - Mode::Deferred - Default SQLite transaction behavior
    - Mode::Immediate - Reserve write lock immediately
    - Mode::Exclusive - Exclusive lock on database

#### Methods

```cpp
    SqliteTransaction(SqliteDb& sqliteDb, Mode mode = Mode::Immediate)
    void commit() - Commit transaction
    void rollback() - Rollback transaction
    bool inTransaction() const - Check if transaction is active
```

### SqliteValueBinder Class

#### Utility class for binding SqliteValue variants to statements:

```cpp
    SqliteValueBinder(SqliteStatement& stmt)
    void bind(int index, const SqliteValue& value)
```
	
## Building

### Prerequisites
- CMake 3.15 or higher
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)

### Basic Build
```bash
# Clone the repository
git clone <your-repo-url>
cd sqlitedb

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the library
make
```

### Build Options

The project supports several SQLite compilation options that can be enabled during configuration:

```bash
# Enable specific SQLite features
cmake -DSQLITE_ENABLE_FTS5=ON -DSQLITE_ENABLE_JSON1=ON -DSQLITE_ENABLE_RTREE=ON ..

# Disable deprecated APIs for smaller binary size
cmake -DSQLITE_OMIT_DEPRECATED=ON ..

# Build with all features enabled
cmake \
  -DSQLITE_ENABLE_FTS5=ON \
  -DSQLITE_ENABLE_JSON1=ON \
  -DSQLITE_ENABLE_COLUMN_METADATA=ON \
  -DSQLITE_ENABLE_LOAD_EXTENSION=ON \
  ..
```

### Available Options:

    - SQLITE_ENABLE_FTS5 (ON by default): Enable Full-Text Search version 5
    - SQLITE_ENABLE_FTS4 (OFF by default): Enable Full-Text Search version 4
    - SQLITE_ENABLE_FTS3 (OFF by default): Enable Full-Text Search version 3
    - SQLITE_ENABLE_RTREE (OFF by default): Enable R*Tree spatial indexing
    - SQLITE_ENABLE_JSON1 (ON by default): Enable JSON extension
    - SQLITE_ENABLE_COLUMN_METADATA (ON by default): Enable column metadata access
    - SQLITE_ENABLE_LOAD_EXTENSION (ON by default): Enable loadable extensions
    - SQLITE_OMIT_DEPRECATED (ON by default): Omit deprecated SQLite APIs
	
### Build Types

By default, the project builds in Release mode. You can specify other build types:

```bash
# Debug build with symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release with debug info
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# Minimum size release
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
```

## License

MIT License - see [MIT License website](https://opensource.org/license/mit) for details.