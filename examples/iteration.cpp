#include <choochoo/json.hpp>
#include <iostream>

int main() {
    std::string json = R"({
        "fruits": ["apple", "banana", "cherry"],
        "prices": { "apple": 1.2, "banana": 0.8, "cherry": 2.5 }
    })";

    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    if (!result) {
        std::cerr << "Parse error: " << result.error() << std::endl;
        return 1;
    }

    auto root = result.value();
    auto fruits_opt = root.as_object()->get().at("fruits").as_array();
    if (fruits_opt) {
        const auto& fruits = fruits_opt->get();
        std::cout << "Fruits: ";
        for (const auto& fruit : fruits) {
            auto str_opt = fruit.as_string();
            if (str_opt) {
                std::cout << str_opt->get() << " ";
            }
        }
        std::cout << std::endl;
    }

    auto prices_opt = root.as_object()->get().at("prices").as_object();
    if (prices_opt) {
        const auto& prices = prices_opt->get();
        std::cout << "Prices:\n";
        for (const auto& [fruit, price] : prices) {
            auto num_opt = price.as_number();
            if (num_opt) {
                std::cout << "  " << fruit << ": " << num_opt.value() << std::endl;
            }
        }
    }

    return 0;
}
