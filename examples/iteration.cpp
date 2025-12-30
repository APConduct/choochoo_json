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
        "fruits": ["apple", "banana", "cherry"],
        "prices": { "apple": 1.2, "banana": 0.8, "cherry": 2.5 }
    })";

    choochoo::json::Lexer lexer(json);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    if (!result) {
        std::cerr << "Parse error: " << result.error() << '\n';
        return 1;
    }

    auto root = result.value();
    const auto& obj = root.as_object()->get();
    const std::string* fruits_kptr = find_key(obj, "fruits");
    auto fruits_opt = fruits_kptr ? obj.at(fruits_kptr).as_array() : std::nullopt;
    if (fruits_opt) {
        const auto& fruits = fruits_opt->get();
        std::cout << "Fruits: ";
        for (const auto& fruit : fruits) {
            auto str_opt = fruit.as_string();
            if (str_opt) {
                std::cout << str_opt->get() << " ";
            }
        }
        std::cout << '\n';
    }

    const std::string* prices_kptr = find_key(obj, "prices");
    auto prices_opt = prices_kptr ? obj.at(prices_kptr).as_object() : std::nullopt;
    if (prices_opt) {
        const auto& prices = prices_opt->get();
        std::cout << "Prices:\n";
        for (const auto& [fruit_kptr, price] : prices) {
            auto num_opt = price.as_number();
            if (num_opt) {
                std::cout << "  " << *fruit_kptr << ": " << num_opt.value() << '\n';
            }
        }
    }

    return 0;
}
