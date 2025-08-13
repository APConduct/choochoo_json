#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <unordered_map>
#include "choochoo/json.hpp"

// Helper to find interned key pointer in object map
const std::string* find_key(const std::unordered_map<const std::string*, choochoo::json::Value>& obj,
                            const std::string& key) {
    for (const auto& [kptr, _] : obj) {
        if (*kptr == key)
            return kptr;
    }
    return nullptr;
}

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

    const std::string* name_kptr = find_key(obj, "name");
    REQUIRE(name_kptr);
    REQUIRE(obj.at(name_kptr).as_string()->get() == "Streamy");

    const std::string* age_kptr = find_key(obj, "age");
    REQUIRE(age_kptr);
    REQUIRE(obj.at(age_kptr).as_number().value() == 99);

    const std::string* active_kptr = find_key(obj, "active");
    REQUIRE(active_kptr);
    REQUIRE(obj.at(active_kptr).as_boolean().value() == false);

    const std::string* scores_kptr = find_key(obj, "scores");
    REQUIRE(scores_kptr);
    auto scores_opt = obj.at(scores_kptr).as_array();
    REQUIRE(scores_opt.has_value());
    const auto& scores = scores_opt->get();
    REQUIRE(scores.size() == 3);
    REQUIRE(scores[0].as_number().value() == 10);
    REQUIRE(scores[1].as_number().value() == 20);
    REQUIRE(scores[2].as_number().value() == 30);

    const std::string* meta_kptr = find_key(obj, "meta");
    REQUIRE(meta_kptr);
    auto meta_opt = obj.at(meta_kptr).as_object();
    REQUIRE(meta_opt.has_value());
    const auto& meta = meta_opt->get();
    const std::string* created_kptr = find_key(meta, "created");
    REQUIRE(created_kptr);
    REQUIRE(meta.at(created_kptr).as_string()->get() == "2024");
    const std::string* verified_kptr = find_key(meta, "verified");
    REQUIRE(verified_kptr);
    REQUIRE(meta.at(verified_kptr).as_boolean().value() == true);
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

    const std::string* empty_obj_kptr = find_key(obj, "empty_obj");
    REQUIRE(empty_obj_kptr);
    REQUIRE(obj.at(empty_obj_kptr).type() == choochoo::json::Type::OBJECT);
    REQUIRE(obj.at(empty_obj_kptr).as_object()->get().empty());

    const std::string* empty_arr_kptr = find_key(obj, "empty_arr");
    REQUIRE(empty_arr_kptr);
    REQUIRE(obj.at(empty_arr_kptr).type() == choochoo::json::Type::ARRAY);
    REQUIRE(obj.at(empty_arr_kptr).as_array()->get().empty());
}
