cmake_minimum_required(VERSION 3.10.0)
project(mrjk-web-server)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log iostreams)
message(STATUS "Boost version: ${Boost_VERSION}")

set(LibXml2_USE_STATIC_LIBS ON)

find_package(LibXml2 REQUIRED)
find_package(ZLIB REQUIRED)
# include_directories(bigtable/include) # Uncomment to use the bigtable library
# find_package(bigtable_client REQUIRED) # Uncomment to use the bigtable library -> Add to target_link_libraries (bigtable_client) | Add to add_library (bigtable/src/blog_bigtable.cc)

set(PQXX /usr/local/include/pqxx)
find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

include_directories(include)
include_directories(${LIBXML2_INCLUDE_DIRS})

# Update name and srcs - ** we'll need to update these after refactoring
add_library(session_server_lib src/session.cc src/server.cc src/NginxConfigParser.cc src/request_parser.cc src/response_helper_library.cc src/static_request_handler.cc  src/echo_request_handler.cc src/request_dispatcher.cc src/error_404_request_handler.cc src/status_request_handler.cc src/proxy_request_handler.cc src/redirect_request_handler.cc src/response_parser.cc src/health_request_handler.cc src/blog_database.cc src/upload_form_request_handler.cc src/blog_upload_request_handler.cc)
add_library(mock_database_lib src/mock_database.cc)

# Update executable name, srcs, and deps
add_executable(webserver src/server_main.cc)
target_link_libraries(webserver session_server_lib Boost::system Boost::log_setup Boost::log Boost::iostreams ZLIB::ZLIB ${LIBXML2_LIBRARY} ${PQXX_LIB} ${PQ_LIB})

# Update test executable name, srcs, and deps
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test session_server_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::iostreams ZLIB::ZLIB ${LIBXML2_LIBRARY})

add_executable(request_parser_handler_test tests/request_handler_test.cc tests/request_parser_test.cc)
target_link_libraries(request_parser_handler_test session_server_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::iostreams ZLIB::ZLIB ${LIBXML2_LIBRARY})

add_executable(request_handler_proxy_test tests/request_handler_proxy_test.cc)
target_link_libraries(request_handler_proxy_test session_server_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::iostreams ZLIB::ZLIB ${LIBXML2_LIBRARY})

add_executable(response_test tests/response_test.cc)
target_link_libraries(response_test session_server_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::iostreams ZLIB::ZLIB ${LIBXML2_LIBRARY})

add_executable(response_parser_test tests/response_parser_test.cc)
target_link_libraries(response_parser_test session_server_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::iostreams ZLIB::ZLIB ${LIBXML2_LIBRARY})

add_executable(request_handler_health_test tests/request_handler_health_test.cc)
target_link_libraries(request_handler_health_test session_server_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::iostreams ZLIB::ZLIB ${LIBXML2_LIBRARY})

add_executable(request_handler_blog_upload_test tests/request_handler_blog_upload_test.cc)
target_link_libraries(request_handler_blog_upload_test session_server_lib mock_database_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::iostreams ZLIB::ZLIB ${LIBXML2_LIBRARY} ${PQXX_LIB} ${PQ_LIB})

add_executable(mock_database_test tests/mock_database_test.cc)
target_link_libraries(mock_database_test mock_database_lib gtest_main Boost::log)

# Update with test binary
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_parser_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_proxy_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(response_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(response_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_health_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_blog_upload_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(mock_database_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME integration_test COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test.sh)
add_test(NAME multithreading_test COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tests/multithreading_test.py)

# Update with target/test targets
include(cmake/CodeCoverageReportConfig.cmake)

generate_coverage_report(TARGETS webserver session_server_lib TESTS config_parser_test request_parser_handler_test request_handler_proxy_test response_test response_parser_test request_handler_health_test request_handler_blog_upload_test mock_database_test)
