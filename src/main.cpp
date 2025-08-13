
#include <cctype>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include "choochoo/lexer.hpp"
#include "choochoo/parser.hpp"
#include "choochoo/token.hpp"
#include "choochoo/value.hpp"

int main() {
    std::string json_input = R"({
            "name": "Perry",
            "age": 20,
            "active": true,
            "scores": [95, 87, 92],
            "metadata": {"created": "2025", "version": 1.2}
        })";

    try {
        choochoo::json::Lexer lexer(json_input);

        choochoo::json::Parser parser(lexer);

        auto root = parser.parse();

        // Check for EOF after parsing root value
        if (parser.current_token().type_ != choochoo::json::token::Type::EOF_TOKEN) {
            throw std::runtime_error("Extra data after root JSON value");
        }

        std::cout << "Successfully parsed JSON!" << std::endl;
        std::cout << "Root type: " << static_cast<int>(root.type()) << std::endl;

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
    catch (const std::exception& e) {
        std::cerr << "Parser error: " << e.what() << '\n';
    }
}
