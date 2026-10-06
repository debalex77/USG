# QSQLCIPHER – driver SQL Qt 6 bazat pe SQLCipher (derivat din qsqlite, Qt 6.9.3)
cmake_minimum_required(VERSION 3.21)
project(qsqlcipher VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Sursa SQLCipher:
#   SQLCIPHER_AMALGAMATION_DIR = director cu sqlite3.c + sqlite3.h generate de SQLCipher
#                                (compilat direct în plugin – recomandat)
#   altfel                     = libsqlcipher de sistem prin pkg-config (apt: libsqlcipher-dev)
set(SQLCIPHER_AMALGAMATION_DIR "" CACHE PATH "Director cu sqlite3.c/sqlite3.h din SQLCipher")
option(QSQLCIPHER_BUILD_TEST "Construiește programul de test" ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Sql)
# Qt >= 6.10 cere găsirea explicită a modulelor private; în 6.9 ținta există deja
if(NOT TARGET Qt6::SqlPrivate)
    find_package(Qt6 REQUIRED COMPONENTS SqlPrivate)
endif()
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

qt_add_plugin(qsqlcipher
    CLASS_NAME QSQLCipherDriverPlugin
    PLUGIN_TYPE sqldrivers
)
target_sources(qsqlcipher PRIVATE
    src/qsql_sqlcipher.cpp src/qsql_sqlcipher_p.h
    src/qsql_sqlcipher_vfs.cpp src/qsql_sqlcipher_vfs_p.h
    src/smain.cpp
    src/sqlcipher.json
)

target_compile_definitions(qsqlcipher PRIVATE
    QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_CONTEXTLESS_CONNECT
    QT_USE_QSTRINGBUILDER   # ca la compilarea internă a Qt (operator+ pe QStringView)
    SQLITE_HAS_CODEC
)
target_link_libraries(qsqlcipher PRIVATE Qt6::Core Qt6::CorePrivate Qt6::Sql Qt6::SqlPrivate)

if(SQLCIPHER_AMALGAMATION_DIR)
    if(NOT EXISTS "${SQLCIPHER_AMALGAMATION_DIR}/sqlite3.c")
        message(FATAL_ERROR "Nu găsesc ${SQLCIPHER_AMALGAMATION_DIR}/sqlite3.c")
    endif()
    message(STATUS "SQLCipher: amalgamare din ${SQLCIPHER_AMALGAMATION_DIR}")
    target_sources(qsqlcipher PRIVATE "${SQLCIPHER_AMALGAMATION_DIR}/sqlite3.c")
    target_include_directories(qsqlcipher PRIVATE "${SQLCIPHER_AMALGAMATION_DIR}")
    target_compile_definitions(qsqlcipher PRIVATE
        # obligatorii pentru SQLCipher 4.7+
        SQLITE_EXTRA_INIT=sqlcipher_extra_init
        SQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown
        SQLITE_TEMP_STORE=2
        SQLITE_THREADSAFE=1
        # aceleași facilități ca în qsqlite-ul oficial
        SQLITE_ENABLE_COLUMN_METADATA
        SQLITE_ENABLE_FTS3 SQLITE_ENABLE_FTS3_PARENTHESIS SQLITE_ENABLE_FTS4 SQLITE_ENABLE_FTS5
        SQLITE_ENABLE_GEOPOLY SQLITE_ENABLE_JSON1 SQLITE_ENABLE_MATH_FUNCTIONS SQLITE_ENABLE_RTREE
        SQLITE_OMIT_COMPLETE
        $<$<CONFIG:Release>:NDEBUG>
    )
    if(NOT WIN32)
        # funcții POSIX; pe Windows SQLite folosește os_win.c și localtime_s
        target_compile_definitions(qsqlcipher PRIVATE
            HAVE_USLEEP=1 HAVE_LOCALTIME_R=1 HAVE_POSIX_FALLOCATE=1)
    endif()
    set_source_files_properties("${SQLCIPHER_AMALGAMATION_DIR}/sqlite3.c"
        PROPERTIES COMPILE_OPTIONS "-w")
    find_package(Threads REQUIRED)
    target_link_libraries(qsqlcipher PRIVATE OpenSSL::Crypto ${CMAKE_DL_LIBS} Threads::Threads)
    if(WIN32)
        # dependențele OpenSSL static (aceleași ca în qmake/modules/dependencies.pri)
        target_link_libraries(qsqlcipher PRIVATE crypt32 ws2_32 advapi32 user32)
    else()
        target_link_libraries(qsqlcipher PRIVATE m)
    endif()
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SQLCIPHER REQUIRED IMPORTED_TARGET sqlcipher)
    message(STATUS "SQLCipher: biblioteca de sistem ${SQLCIPHER_VERSION}")
    target_link_libraries(qsqlcipher PRIVATE PkgConfig::SQLCIPHER OpenSSL::Crypto)
endif()

# Simbolurile sqlite3_* rămân interne pluginului: nu intră în conflict
# cu qsqlite sau cu alte copii de SQLite încărcate în același proces.
set_target_properties(qsqlcipher PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
if(NOT WIN32)
    # pe Windows DLL-ul exportă doar simbolurile marcate dllexport
    target_link_options(qsqlcipher PRIVATE "LINKER:--exclude-libs,ALL")
endif()

# Instalare direct în kit-ul Qt: <Qt>/6.9.3/<kit>/plugins/sqldrivers
get_target_property(_qmake Qt6::qmake IMPORTED_LOCATION)
execute_process(COMMAND "${_qmake}" -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_PLUGINS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
set(QSQLCIPHER_INSTALL_DIR "${QT_PLUGINS_DIR}/sqldrivers" CACHE PATH "Unde se instalează pluginul")
install(TARGETS qsqlcipher LIBRARY DESTINATION "${QSQLCIPHER_INSTALL_DIR}")

if(QSQLCIPHER_BUILD_TEST)
    qt_add_executable(qsqlcipher_test test/main.cpp)
    target_link_libraries(qsqlcipher_test PRIVATE Qt6::Core Qt6::Sql)
    # testul găsește pluginul în build/plugins/sqldrivers fără instalare
    set_target_properties(qsqlcipher PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/sqldrivers")
    target_compile_definitions(qsqlcipher_test PRIVATE
        QSQLCIPHER_BUILD_PLUGINS="${CMAKE_BINARY_DIR}/plugins")
endif()
