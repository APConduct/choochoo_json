#pragma once
#include <string_view>

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
        std::string_view value;
        size_t line{};
        size_t column{};
    };
} // namespace choochoo::json
