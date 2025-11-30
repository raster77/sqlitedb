cmake_minimum_required(VERSION 3.15)
project(SQLiteDb VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(SQLITE_ENABLE_FTS5 "Enable FTS5 support" ON)
option(SQLITE_ENABLE_FTS4 "Enable FTS4 support" OFF)
option(SQLITE_ENABLE_FTS3 "Enable FTS3 support" OFF)
option(SQLITE_ENABLE_RTREE "Enable R*Tree support" OFF)
option(SQLITE_ENABLE_JSON1 "Enable JSON1 extension" ON)
option(SQLITE_ENABLE_COLUMN_METADATA "Enable column metadata" ON)
option(SQLITE_ENABLE_LOAD_EXTENSION "Enable loadable extensions" ON)
option(SQLITE_OMIT_DEPRECATED "Omit deprecated APIs and pragmas" ON)

set(SQLITE_SRC sqlite/sqlite3.c)

file(GLOB_RECURSE SQLITEDB_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_library(sqlitedb STATIC
    ${SQLITEDB_SOURCES}
    ${SQLITE_SRC}
)

target_include_directories(sqlitedb
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sqlite>
)

set(SQLITE_DEFINITIONS
    SQLITE_THREADSAFE=1
    SQLITE_DEFAULT_MEMSTATUS=0
    SQLITE_USE_URI=1
    SQLITE_CORE=1
)

if(SQLITE_ENABLE_FTS5)
	list(APPEND SQLITE_DEFINITIONS
		SQLITE_ENABLE_FTS5
		SQLITE_ENABLE_FTS5_TRIGRAM_TOKENIZER
	)
endif()

if(SQLITE_ENABLE_FTS4)
    list(APPEND SQLITE_DEFINITIONS
		SQLITE_ENABLE_FTS4
	)
endif()

if(SQLITE_ENABLE_FTS3)
    list(APPEND SQLITE_DEFINITIONS
		SQLITE_ENABLE_FTS3
	)
endif()

if(SQLITE_ENABLE_RTREE)
    list(APPEND SQLITE_DEFINITIONS
		SQLITE_ENABLE_RTREE
	)
endif()

if(SQLITE_ENABLE_JSON1)
    list(APPEND SQLITE_DEFINITIONS
		SQLITE_ENABLE_JSON1
	)
endif()

if(SQLITE_ENABLE_COLUMN_METADATA)
    list(APPEND SQLITE_DEFINITIONS
		SQLITE_ENABLE_COLUMN_METADATA
	)
endif()

if(SQLITE_ENABLE_LOAD_EXTENSION)
    list(APPEND SQLITE_DEFINITIONS
		SQLITE_ENABLE_LOAD_EXTENSION
	)
endif()

if(SQLITE_OMIT_DEPRECATED)
    list(APPEND SQLITE_DEFINITIONS
		SQLITE_OMIT_DEPRECATED
	)
endif()

target_compile_definitions(sqlitedb PRIVATE ${SQLITE_DEFINITIONS})

if(MSVC)
    target_compile_options(sqlitedb PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(sqlitedb PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(sqlitedb PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

set_target_properties(sqlitedb PROPERTIES
    CXX_EXTENSIONS OFF
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

option(INSTALL_LIBRARY "Enable installation rules" OFF)

if(INSTALL_LIBRARY)
    include(GNUInstallDirs)
    install(TARGETS sqlitedb
        EXPORT sqlitedbTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    install(DIRECTORY sqlitedb/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sqlitedb
    )
    
    install(EXPORT sqlitedbTargets
        FILE sqlitedbTargets.cmake
        NAMESPACE sdb::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sqlitedb
    )
endif()
