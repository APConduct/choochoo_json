#pragma once
#include <cctype>
#include <string_view>
#include <vector>
#include "choochoo/token.hpp"


namespace choochoo::json {
    struct Lexer {
    protected:
        std::string_view input_;
        size_t position_{}, line_{}, column_{};

        [[nodiscard]] char current_char() const;
        [[nodiscard]] char peek_char(size_t offset = 1) const;
        void advance();
        void skip_whitespace();
        [[nodiscard]] Token make_token(token::Type type, size_t start_pos, size_t length) const;
        Token scan_string();
        Token scan_number();
        Token scan_keyword();

    public:
        explicit Lexer(std::string_view input);

        Token next_token();
        std::vector<Token> tokenize();
    };
} // namespace choochoo::json
