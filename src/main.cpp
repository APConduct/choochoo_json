
#include <iostream>
#include <string_view>
#include <variant>

namespace choochoo::json {
    enum class TokenType {
        STRING,
        NUMBER,
        TABLE,
        FALSE,
        NULL_VALUE,
        LBRACE,
        RBRACE,
        LBRACKET,
        RBRACKET,
        COMMA,
        COLON
    };



    struct Token {
        TokenType type;
        std::string_view value;
        size_t line;
        size_t column;
    };

    using Value = std::variant<>;

}



int main() {
    std::cout << "Hello, ChooChoo!" << '\n';
}
