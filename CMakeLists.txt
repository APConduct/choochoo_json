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



add_executable(choochoo_json
    src/main.cpp
)

target_include_directories(choochoo_json
    PRIVATE
        include
)

# Add Catch2 test target
add_executable(choochoo_json_tests
    tests/test_parser.cpp
)
target_include_directories(choochoo_json_tests
    PRIVATE
        include
)
target_link_libraries(choochoo_json_tests PRIVATE Catch2::Catch2WithMain)
