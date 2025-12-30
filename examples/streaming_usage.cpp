#include <choochoo/json.hpp>
#include <iostream>
#include <sstream>
#include <string>
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
    // Simulate streaming input using std::istringstream
    std::string json_input = R"({
        "streamed": true,
        "numbers": [10, 20, 30],
        "info": {
            "source": "istringstream",
            "valid": true
        }
    })";

    std::istringstream stream(json_input);

    // Use the streaming constructor for Lexer
    choochoo::json::Lexer lexer(stream);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    if (!result) {
        std::cerr << "Streaming parse error: " << result.error() << '\n';
        return 1;
    }

    auto root = result.value();
    std::cout << "Successfully parsed JSON from stream!\n";
    std::cout << "Pretty JSON:\n" << root.pretty() << '\n';

    // Access streamed values
    auto obj_opt = root.as_object();
    if (obj_opt) {
        const auto& obj = obj_opt->get();
        const std::string* streamed_kptr = find_key(obj, "streamed");
        if (streamed_kptr) {
            auto streamed_val = obj.at(streamed_kptr).as_boolean();
            std::cout << "streamed: " << (streamed_val.value() ? "true" : "false") << '\n';
        }
        const std::string* numbers_kptr = find_key(obj, "numbers");
        if (numbers_kptr) {
            const auto& numbers_val = obj.at(numbers_kptr);
            if (numbers_val.type() == choochoo::json::Type::ARRAY) {
                std::cout << "numbers: ";
                for (const auto& num : numbers_val) {
                    auto n = num.as_number();
                    if (n)
                        std::cout << *n << " ";
                }
                std::cout << '\n';
            }
        }
        const std::string* info_kptr = find_key(obj, "info");
        if (info_kptr) {
            const auto& info_val = obj.at(info_kptr);
            auto info_obj = info_val.as_object();
            if (info_obj) {
                const auto& info_map = info_obj->get();
                const std::string* source_kptr = find_key(info_map, "source");
                const std::string* valid_kptr = find_key(info_map, "valid");
                if (source_kptr)
                    std::cout << "info.source: " << info_map.at(source_kptr).as_string()->get() << '\n';
                if (valid_kptr)
                    std::cout << "info.valid: " << (info_map.at(valid_kptr).as_boolean().value() ? "true" : "false")
                              << '\n';
            }
        }
    }

    return 0;
}
