#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
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

TEST_CASE("Array iterator: range-based for and manual iteration") {
    using choochoo::json::Type;
    using choochoo::json::Value;

    std::vector<Value> arr = {Value::number(1), Value::number(2), Value::number(3)};
    Value array_val = Value::array(arr);

    // Range-based for
    std::vector<double> seen;
    for (const auto& v : array_val) {
        auto num = v.as_number();
        REQUIRE(num.has_value());
        seen.push_back(*num);
    }
    REQUIRE(seen == std::vector<double>{1, 2, 3});

    // Manual iterator
    seen.clear();
    for (auto it = array_val.begin(); it != array_val.end(); ++it) {
        auto num = it->as_number();
        REQUIRE(num.has_value());
        seen.push_back(*num);
    }
    REQUIRE(seen == std::vector<double>{1, 2, 3});

    // STL algorithm compatibility
    auto found = std::find_if(array_val.begin(), array_val.end(), [](const Value& v) {
        auto num = v.as_number();
        return num && *num == 2;
    });
    REQUIRE(found != array_val.end());
    REQUIRE(found->as_number().value() == 2);
}

TEST_CASE("Object iterator: range-based for and manual iteration") {
    using choochoo::json::Type;
    using choochoo::json::Value;

    std::unordered_map<const std::string*, Value> obj;
    std::unordered_set<std::string> key_pool;
    for (const auto& k : {"a", "b", "c"}) {
        auto [it, inserted] = key_pool.insert(std::string(k));
        obj.emplace(&(*it), Value::number(k == std::string("a") ? 10 : k == std::string("b") ? 20 : 30));
    }
    Value obj_val = Value::object(obj);

    // Range-based for (via as_object)
    auto obj_opt = obj_val.as_object();
    REQUIRE(obj_opt.has_value());
    std::unordered_set<std::string> keys;
    double sum = 0;
    for (const auto& [keyptr, val] : obj_opt.value().get()) {
        keys.insert(*keyptr);
        auto num = val.as_number();
        REQUIRE(num.has_value());
        sum += *num;
    }
    REQUIRE(keys == std::unordered_set<std::string>{"a", "b", "c"});
    REQUIRE(sum == 60);

    // Manual iterator
    keys.clear();
    sum = 0;
    for (auto it = obj_val.obj_begin(); it != obj_val.obj_end(); ++it) {
        keys.insert(*(it->first));
        auto num = it->second.as_number();
        REQUIRE(num.has_value());
        sum += *num;
    }
    REQUIRE(keys == std::unordered_set<std::string>{"a", "b", "c"});
    REQUIRE(sum == 60);

    // STL algorithm compatibility
    auto found =
        std::find_if(obj_val.obj_begin(), obj_val.obj_end(), [](const auto& pair) { return *pair.first == "b"; });
    REQUIRE(found != obj_val.obj_end());
    REQUIRE(found->second.as_number().value() == 20);
}

TEST_CASE("Iterator throws on wrong type") {
    using choochoo::json::Type;
    using choochoo::json::Value;

    Value num_val = Value::number(42);
    Value str_val = Value::string("hello");

    // Array iterators on non-array
    REQUIRE_THROWS_AS(num_val.begin(), std::logic_error);
    REQUIRE_THROWS_AS(str_val.begin(), std::logic_error);

    // Object iterators on non-object
    REQUIRE_THROWS_AS(num_val.obj_begin(), std::logic_error);
    REQUIRE_THROWS_AS(str_val.obj_begin(), std::logic_error);
}
