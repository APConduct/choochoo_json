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

        [[nodiscard]] char current_char() const { return position_ < input_.size() ? input_[position_] : '\0'; }

        [[nodiscard]] char peek_char(size_t offset = 1) const {
            size_t peek_pos = position_ + offset;
            return peek_pos < input_.size() ? input_[peek_pos] : '\0';
        }

        void advance() {
            if (position_ < input_.size()) {
                if (input_[position_] == '\n') {
                    line_++;
                    column_ = 1;
                }
                else {
                    column_++;
                }
                position_++;
            }
        }

        void skip_whitespace() {
            while (std::isspace(current_char())) {
                advance();
            }
        }

        [[nodiscard]] Token make_token(token::Type type, size_t start_pos, size_t length) const {
            return Token{type, input_.substr(start_pos, length), line_, column_ - length};
        }

        Token scan_string() {
            const size_t quote_pos = position_;
            const size_t start_column = column_;

            advance();
            const size_t start_pos = position_;

            while (current_char() != '"' && current_char() != '\0') {
                if (current_char() == '\\') {
                    advance();
                    if (current_char() == '\0') {
                        break;
                    }
                    advance();
                }
                else {
                    advance();
                }
            }

            if (current_char() != '"') {
                return Token{token::Type::INVALID, input_.substr(quote_pos), line_, start_column + 1};
            }

            size_t length = position_ - start_pos;
            std::string_view str_value = input_.substr(start_pos, length);

            advance();

            return Token{token::Type::STRING, str_value, line_, start_column};
        }

        Token scan_number() {
            const size_t start_pos = position_;
            const size_t start_column = column_;

            if (current_char() == '-') {
                advance();
            }

            if (current_char() == '0') {
                advance();
            }
            else if (std::isdigit(current_char())) {
                while (std::isdigit(current_char())) {
                    advance();
                }
            }
            else {
                return Token{token::Type::INVALID, input_.substr(start_pos), line_, start_column};
            }

            if (current_char() == '.') {
                advance();
                if (!std::isdigit(current_char())) {
                    return Token{token::Type::INVALID, input_.substr(start_pos), line_, start_column};
                }
                while (std::isdigit(current_char())) {
                    advance();
                }
            }

            if (current_char() == 'e' || current_char() == 'E') {
                advance();
                if (current_char() == '+' || current_char() == '-') {
                    advance();
                }
                if (!std::isdigit(current_char())) {
                    return Token{token::Type::INVALID, input_.substr(start_pos), line_, start_column};
                }
                while (std::isdigit(current_char())) {
                    advance();
                }
            }

            size_t length = position_ - start_pos;
            return Token{token::Type::NUMBER, input_.substr(start_pos, length), line_, start_column};
        }

        Token scan_keyword() {
            const size_t start_pos = position_;
            const size_t start_column = column_;

            while (std::isalpha(current_char())) {
                advance();
            }

            size_t length = position_ - start_pos;
            std::string_view word = input_.substr(start_pos, length);

            token::Type type;
            if (word == "true") {
                type = token::Type::TRUE;
            }
            else if (word == "false") {
                type = token::Type::FALSE;
            }
            else if (word == "null") {
                type = token::Type::NULL_VALUE;
            }
            else {
                type = token::Type::INVALID;
            }
            return Token{type, word, line_, start_column};
        }

    public:
        explicit Lexer(std::string_view input) : input_(input), position_(0), line_(1), column_(1) {};

        Token next_token() {
            skip_whitespace();

            if (position_ >= input_.size()) {
                return Token{token::Type::EOF_TOKEN, "", line_, column_};
            }

            char ch = current_char();
            size_t current_column = column_;

            switch (ch) {
            case '{':
                advance();
                return Token{token::Type::LBRACE, input_.substr(position_ - 1, 1), line_, current_column};
            case '}':
                advance();
                return Token{token::Type::RBRACE, input_.substr(position_ - 1, 1), line_, current_column};
            case '[':
                advance();
                return Token{token::Type::LBRACKET, input_.substr(position_ - 1, 1), line_, current_column};
            case ']':
                advance();
                return Token{token::Type::RBRACKET, input_.substr(position_ - 1, 1), line_, current_column};
            case ',':
                advance();
                return Token{token::Type::COMMA, input_.substr(position_ - 1, 1), line_, current_column};
            case ':':
                advance();
                return Token{token::Type::COLON, input_.substr(position_ - 1, 1), line_, current_column};
            case '"':
                return scan_string();
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return scan_number();
            default:
                if (std::isalpha(ch)) {
                    return scan_keyword();
                }
                else {
                    advance();
                    return Token{token::Type::INVALID, input_.substr(position_ - 1, 1), line_, current_column};
                }
            }
        }

        std::vector<Token> tokenize() {
            std::vector<Token> tokens;
            Token token;

            do {
                token = next_token();
                tokens.push_back(token);
            }
            while (token.type_ != token::Type::EOF_TOKEN && token.type_ != token::Type::INVALID);

            return tokens;
        }
    };
} // namespace choochoo::json
