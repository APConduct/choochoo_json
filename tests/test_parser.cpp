#include <catch2/catch_test_macros.hpp>
#include "choochoo/lexer.hpp"
#include "choochoo/parser.hpp"

TEST_CASE("Valid JSON parses successfully") {
    std::string json =
        R"({"name": "Alice", "age": 30, "active": true, "scores": [100, 99], "meta": {"created": "2024"}})";
    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);
    auto result = parser.parse();
    REQUIRE(result);
    REQUIRE(result.value().type() == choochoo::json::Type::OBJECT);
}

TEST_CASE("Missing quotes on key fails") {
    std::string json = R"({name: "Alice", "age": 30})";
    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);
    auto result = parser.parse();
    REQUIRE_FALSE(result);
}

TEST_CASE("Missing value fails") {
    std::string json = R"({"name": "Alice", "age": })";
    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);
    auto result = parser.parse();
    REQUIRE_FALSE(result);
}

TEST_CASE("Trailing comma in array fails") {
    std::string json = R"([1, 2, 3,])";
    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);
    auto result = parser.parse();
    REQUIRE_FALSE(result);
}

TEST_CASE("Empty object parses successfully") {
    std::string json = R"({})";
    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);
    auto result = parser.parse();
    REQUIRE(result);
    REQUIRE(result.value().type() == choochoo::json::Type::OBJECT);
}

TEST_CASE("Empty array parses successfully") {
    std::string json = R"([])";
    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);
    auto result = parser.parse();
    REQUIRE(result);
    REQUIRE(result.value().type() == choochoo::json::Type::ARRAY);
}

TEST_CASE("Empty input fails") {
    std::string json = "";
    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);
    auto result = parser.parse();
    REQUIRE_FALSE(result);
}

TEST_CASE("Bad string escape fails") {
    std::string json = R"({"name": "Alice\nBad"})";
    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);
    auto result = parser.parse();
    // This should pass unless your parser rejects \n in strings, so let's test a truly bad escape:
    std::string bad_json = R"({"name": "Alice\q"})";
    choochoo::json::Lexer bad_lexer(bad_json);
    choochoo::json::Parser bad_parser(bad_lexer);
    auto bad_result = bad_parser.parse();
    REQUIRE_FALSE(bad_result);
}
