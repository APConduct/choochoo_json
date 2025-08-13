#include <choochoo/json.hpp>
#include <iostream>

int main() {
    // Example of malformed JSON (missing value for "age")
    std::string bad_json = R"({ "name": "Alice", "age": })";
    choochoo::json::Lexer lexer(bad_json);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    if (!result) {
        std::cerr << "Failed to parse JSON:\n";
        std::cerr << "  Error: " << result.error() << std::endl;
        return 1;
    }

    std::cout << "Parsed JSON:\n" << result.value().pretty() << std::endl;
    return 0;
}
