cmake_minimum_required(VERSION 3.28)
project(choochoo_json)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4 # Use the latest stable tag or desired version
)
FetchContent_MakeAvailable(Catch2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



# Library target
add_library(choochoo_json STATIC
    src/value.cpp
    src/parser.cpp
    src/lexer.cpp
    # Add other source files as needed
)

target_include_directories(choochoo_json
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Example executable targets
add_executable(choochoo_json_example
    examples/basic_usage.cpp
)
add_executable(choochoo_json_error_handling
    examples/error_handling.cpp
)
add_executable(choochoo_json_nested_access
    examples/nested_access.cpp
)
add_executable(choochoo_json_iteration
    examples/iteration.cpp
)

add_executable(iterator_usage
    examples/iterator_usage.cpp
)
target_link_libraries(iterator_usage PRIVATE choochoo_json)
target_include_directories(iterator_usage PRIVATE include)

target_link_libraries(choochoo_json_example PRIVATE choochoo_json)
target_include_directories(choochoo_json_example PRIVATE include)

# Install rules for library and headers
install(TARGETS choochoo_json
    EXPORT choochoo_jsonTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/choochoo/ DESTINATION include/choochoo)

target_link_libraries(choochoo_json_error_handling PRIVATE choochoo_json)
target_include_directories(choochoo_json_error_handling PRIVATE include)

target_link_libraries(choochoo_json_nested_access PRIVATE choochoo_json)
target_include_directories(choochoo_json_nested_access PRIVATE include)

target_link_libraries(choochoo_json_iteration PRIVATE choochoo_json)
target_include_directories(choochoo_json_iteration PRIVATE include)

# Add Catch2 test target
add_executable(choochoo_json_tests
    tests/test_parser.cpp
)
target_include_directories(choochoo_json_tests
    PRIVATE
        include
)
target_link_libraries(choochoo_json_tests PRIVATE choochoo_json Catch2::Catch2WithMain)
add_test(NAME choochoo_json_tests COMMAND choochoo_json_tests)

# Add iterator test target
add_executable(choochoo_json_iterator_test
    tests/test_iterator.cpp
)
target_include_directories(choochoo_json_iterator_test
    PRIVATE
        include
)
target_link_libraries(choochoo_json_iterator_test PRIVATE choochoo_json Catch2::Catch2WithMain)

add_test(NAME choochoo_json_iterator_test COMMAND choochoo_json_iterator_test)
