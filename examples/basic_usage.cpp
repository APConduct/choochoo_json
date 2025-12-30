#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include "choochoo/lexer.hpp"
#include "choochoo/parser.hpp"
#include "choochoo/value.hpp"

// Helper to find interned key pointer in object map
const std::string* find_key(const std::unordered_map<const std::string*, choochoo::json::Value>& obj,
                            std::string_view key) {
    for (const auto& [kptr, _] : obj) {
        if (*kptr == key)
            return kptr;
    }
    return nullptr;
}

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

    std::cout << "Successfully parsed JSON!" << '\n';
    std::cout << "Root type: " << static_cast<int>(root.type()) << '\n';

    std::cout << "Pretty JSON:\n" << root.pretty() << '\n';

    // Example access
    if (root.type() == choochoo::json::Type::OBJECT) {
        const auto& obj = root.as_object();
        std::cout << "Root object keys:\n";
        for (const auto& [key, value] : obj->get()) {
            std::cout << "  Key: " << key << ", Type: " << static_cast<int>(value.type()) << "\n";
            if (*key == "name") {
                if (value.type() == choochoo::json::Type::STRING) {
                    std::cout << "  'name' value: " << value.as_string()->get() << "\n";
                }
                else {
                    std::cout << "  'name' is not a string, type: " << static_cast<int>(value.type()) << "\n";
                }

                // --- Iterator usage examples ---

                // If "scores" is an array, iterate using new iterator support
                if (root.type() == choochoo::json::Type::OBJECT) {
                    auto obj_opt = root.as_object();
                    const std::string* scores_kptr = find_key(obj_opt->get(), "scores");
                    if (obj_opt && scores_kptr) {
                        const auto& scores_val = obj_opt->get().at(scores_kptr);
                        if (scores_val.type() == choochoo::json::Type::ARRAY) {
                            std::cout << "Scores (using iterator): ";
                            for (const auto& score : scores_val) {
                                auto num = score.as_number();
                                if (num)
                                    std::cout << *num << " ";
                            }
                            std::cout << '\n';
                        }
                    }
                }

                // Iterate over object keys/values using iterator support
                if (root.type() == choochoo::json::Type::OBJECT) {
                    std::cout << "Iterating over object using obj_begin/obj_end:\n";
                    for (auto it = root.obj_begin(); it != root.obj_end(); ++it) {
                        std::cout << "  [" << *(it->first) << "] type: " << static_cast<int>(it->second.type()) << "\n";
                    }
                }
            }
        }
    }
}
