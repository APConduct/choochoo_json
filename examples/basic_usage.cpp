
#include <cctype>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include "choochoo/lexer.hpp"
#include "choochoo/parser.hpp"
#include "choochoo/value.hpp"

int main() {
    std::string json_input = R"({
    "name": "Perry",
    "age": 20,
    "active": true,
    "scores": [95, 87, 92],
    "metadata": {"created": "2025", "version": 1.2}
})";

    choochoo::json::Lexer lexer(json_input);

    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();

    if (!result) {
        std::cerr << "Parser error: " << result.error() << '\n';
        return 1;
    }

    auto root = result.value();

    std::cout << "Successfully parsed JSON!" << std::endl;
    std::cout << "Root type: " << static_cast<int>(root.type()) << std::endl;

    std::cout << "Pretty JSON:\n" << root.pretty() << std::endl;

    // Example access
    if (root.type() == choochoo::json::Type::OBJECT) {
        const auto& obj = root.as_object();
        std::cout << "Root object keys:\n";
        for (const auto& [key, value] : obj->get()) {
            std::cout << "  Key: " << key << ", Type: " << static_cast<int>(value.type()) << "\n";
            if (key == "name") {
                if (value.type() == choochoo::json::Type::STRING) {
                    std::cout << "  'name' value: " << value.as_string()->get() << "\n";
                }
                else {
                    std::cout << "  'name' is not a string, type: " << static_cast<int>(value.type()) << "\n";
                }
            }
        }
    }
}
