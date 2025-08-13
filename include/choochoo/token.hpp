#pragma once
#include <string>
#include <string_view>
#include <variant>

namespace choochoo::json {
    namespace token {
        enum class Type {
            STRING,
            NUMBER,
            TRUE,
            FALSE,
            NULL_VALUE,
            LBRACE,
            RBRACE,
            LBRACKET,
            RBRACKET,
            COMMA,
            COLON,
            EOF_TOKEN,
            INVALID
        };
    }

    struct Token {
        token::Type type_{};
        std::variant<std::string_view, std::string> value;
        size_t line{};
        size_t column{};
    };
} // namespace choochoo::json
