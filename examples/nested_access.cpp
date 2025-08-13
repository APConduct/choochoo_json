#include <choochoo/json.hpp>
#include <iostream>
#include <unordered_map>

// Helper to find interned key pointer in object map
const std::string* find_key(const std::unordered_map<const std::string*, choochoo::json::Value>& obj,
                            const std::string& key) {
    for (const auto& [kptr, _] : obj) {
        if (*kptr == key)
            return kptr;
    }
    return nullptr;
}

int main() {
    std::string json = R"({
        "user": {
            "name": "Alice",
            "age": 30,
            "scores": [100, 98, 95]
        }
    })";

    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    if (!result) {
        std::cerr << "Parse error: " << result.error() << std::endl;
        return 1;
    }

    auto root = result.value();
    auto user_obj_opt = root.as_object();
    if (!user_obj_opt) {
        std::cerr << "Root is not an object." << std::endl;
        return 1;
    }
    const auto& obj = user_obj_opt->get();
    const std::string* user_kptr = find_key(obj, "user");
    auto user_obj = user_kptr ? obj.at(user_kptr).as_object() : std::nullopt;
    if (user_obj) {
        const auto& user_map = user_obj->get();
        const std::string* name_kptr = find_key(user_map, "name");
        const std::string* age_kptr = find_key(user_map, "age");
        const std::string* scores_kptr = find_key(user_map, "scores");
        if (name_kptr)
            std::cout << "Name: " << user_map.at(name_kptr).as_string()->get() << std::endl;
        if (age_kptr)
            std::cout << "Age: " << user_map.at(age_kptr).as_number().value() << std::endl;
        auto scores = scores_kptr ? user_map.at(scores_kptr).as_array() : std::nullopt;
        if (scores) {
            std::cout << "Scores: ";
            for (const auto& score : scores->get()) {
                std::cout << score.as_number().value() << " ";
            }
            std::cout << std::endl;
        }
    }
    else {
        std::cerr << "'user' is not an object." << std::endl;
        return 1;
    }
    return 0;
}
