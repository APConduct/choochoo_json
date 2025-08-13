cmake_minimum_required(VERSION 3.28)
project(choochoo_json)

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

# Example executable target
add_executable(choochoo_json_example
    examples/basic_usage.cpp
)

target_link_libraries(choochoo_json_example PRIVATE choochoo_json)
target_include_directories(choochoo_json_example PRIVATE include)

# Add Catch2 test target
add_executable(choochoo_json_tests
    tests/test_parser.cpp
)
target_include_directories(choochoo_json_tests
    PRIVATE
        include
)
target_link_libraries(choochoo_json_tests PRIVATE choochoo_json Catch2::Catch2WithMain)
