#include <choochoo/json.hpp>
#include <iostream>

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
    const auto& user_obj = user_obj_opt->get().at("user").as_object();
    if (user_obj) {
        std::cout << "Name: " << user_obj->get().at("name").as_string()->get() << std::endl;
        std::cout << "Age: " << user_obj->get().at("age").as_number().value() << std::endl;
        auto scores = user_obj->get().at("scores").as_array();
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
