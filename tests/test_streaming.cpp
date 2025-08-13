#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "choochoo/json.hpp"

TEST_CASE("Streaming lexer/parser parses valid JSON from std::istringstream") {
    std::string json = R"({
        "name": "Streamy",
        "age": 99,
        "active": false,
        "scores": [10, 20, 30],
        "meta": {"created": "2024", "verified": true}
    })";
    std::istringstream stream(json);

    choochoo::json::Lexer lexer(stream);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    REQUIRE(result);
    auto root = result.value();
    REQUIRE(root.type() == choochoo::json::Type::OBJECT);

    auto obj_opt = root.as_object();
    REQUIRE(obj_opt.has_value());
    const auto& obj = obj_opt->get();

    REQUIRE(obj.at("name").as_string()->get() == "Streamy");
    REQUIRE(obj.at("age").as_number().value() == 99);
    REQUIRE(obj.at("active").as_boolean().value() == false);

    auto scores_opt = obj.at("scores").as_array();
    REQUIRE(scores_opt.has_value());
    const auto& scores = scores_opt->get();
    REQUIRE(scores.size() == 3);
    REQUIRE(scores[0].as_number().value() == 10);
    REQUIRE(scores[1].as_number().value() == 20);
    REQUIRE(scores[2].as_number().value() == 30);

    auto meta_opt = obj.at("meta").as_object();
    REQUIRE(meta_opt.has_value());
    const auto& meta = meta_opt->get();
    REQUIRE(meta.at("created").as_string()->get() == "2024");
    REQUIRE(meta.at("verified").as_boolean().value() == true);
}

TEST_CASE("Streaming lexer/parser fails on malformed JSON") {
    std::string bad_json = R"({ "name": "Streamy", "age": })";
    std::istringstream stream(bad_json);

    choochoo::json::Lexer lexer(stream);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    REQUIRE_FALSE(result);
    bool found_unexpected = result.error().find("Unexpected token") != std::string::npos;
    bool found_no_value = result.error().find("No value to parse") != std::string::npos;
    if (!(found_unexpected || found_no_value)) {
        FAIL("Error message did not contain expected substrings");
    }
}

TEST_CASE("Streaming lexer/parser parses empty array and object") {
    std::string json = R"({"empty_obj": {}, "empty_arr": []})";
    std::istringstream stream(json);

    choochoo::json::Lexer lexer(stream);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    REQUIRE(result);
    auto root = result.value();
    REQUIRE(root.type() == choochoo::json::Type::OBJECT);

    auto obj_opt = root.as_object();
    REQUIRE(obj_opt.has_value());
    const auto& obj = obj_opt->get();

    REQUIRE(obj.at("empty_obj").type() == choochoo::json::Type::OBJECT);
    REQUIRE(obj.at("empty_obj").as_object()->get().empty());

    REQUIRE(obj.at("empty_arr").type() == choochoo::json::Type::ARRAY);
    REQUIRE(obj.at("empty_arr").as_array()->get().empty());
}
