#include <choochoo/json.hpp>
#include <iostream>
#include <sstream>
#include <string>

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
        std::cerr << "Streaming parse error: " << result.error() << std::endl;
        return 1;
    }

    auto root = result.value();
    std::cout << "Successfully parsed JSON from stream!\n";
    std::cout << "Pretty JSON:\n" << root.pretty() << std::endl;

    // Access streamed values
    auto obj_opt = root.as_object();
    if (obj_opt) {
        const auto& obj = obj_opt->get();
        if (obj.count("streamed")) {
            auto streamed_val = obj.at("streamed").as_boolean();
            std::cout << "streamed: " << (streamed_val.value() ? "true" : "false") << std::endl;
        }
        if (obj.count("numbers")) {
            const auto& numbers_val = obj.at("numbers");
            if (numbers_val.type() == choochoo::json::Type::ARRAY) {
                std::cout << "numbers: ";
                for (const auto& num : numbers_val) {
                    auto n = num.as_number();
                    if (n)
                        std::cout << *n << " ";
                }
                std::cout << std::endl;
            }
        }
        if (obj.count("info")) {
            const auto& info_val = obj.at("info");
            auto info_obj = info_val.as_object();
            if (info_obj) {
                std::cout << "info.source: " << info_obj->get().at("source").as_string()->get() << std::endl;
                std::cout << "info.valid: " << (info_obj->get().at("valid").as_boolean().value() ? "true" : "false")
                          << std::endl;
            }
        }
    }

    return 0;
}
