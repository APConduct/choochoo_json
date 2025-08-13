#include "choochoo/lexer.hpp"
#include <deque>
#include <iostream>
#include <string>

namespace choochoo::json {

    char Lexer::current_char() const {
        if (using_stream_) {
            // Try to refill buffer if empty
            if (stream_buffer_.empty() && input_stream_ && input_stream_->good()) {
                int next = input_stream_->get();
                if (next != std::char_traits<char>::eof()) {
                    // Remove constness for stream_buffer_ (make it mutable in class definition)
                    const_cast<std::deque<char>&>(stream_buffer_).push_back(static_cast<char>(next));
                }
            }
            if (stream_buffer_.empty()) {
                return '\0';
            }
            return stream_buffer_.front();
        }
        return position_ < input_.size() ? input_[position_] : '\0';
    }

    char Lexer::peek_char(size_t offset) {
        if (using_stream_) {
            // Fill buffer if needed
            while (stream_buffer_.size() <= offset && input_stream_ && input_stream_->good()) {
                int next = input_stream_->get();
                if (next == std::char_traits<char>::eof())
                    break;
                stream_buffer_.push_back(static_cast<char>(next));
            }
            if (stream_buffer_.size() > offset) {
                return stream_buffer_[offset];
            }
            return '\0';
        }
        size_t peek_pos = position_ + offset;
        return peek_pos < input_.size() ? input_[peek_pos] : '\0';
    }

    void Lexer::advance() {
        if (using_stream_) {
            if (!stream_buffer_.empty()) {
                char ch = stream_buffer_.front();
                stream_buffer_.pop_front();
                if (ch == '\n') {
                    stream_line_++;
                    stream_column_ = 1;
                }
                else {
                    stream_column_++;
                }
            }
            // Always refill buffer after popping
            while (stream_buffer_.size() < 1 && input_stream_ && input_stream_->good()) {
                int next = input_stream_->get();
                if (next == std::char_traits<char>::eof())
                    break;
                stream_buffer_.push_back(static_cast<char>(next));
            }
        }
        else {
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
    }

    void Lexer::skip_whitespace() {
        while (std::isspace(current_char())) {
            advance();
        }
    }

    Token Lexer::make_token(token::Type type, size_t start_pos, size_t length) const {
        if (using_stream_) {
            // For streaming, we can't provide a string_view into the stream, so we use a placeholder
            // (You may want to buffer the token value in a real implementation)
            return Token{type, std::string(""), stream_line_, stream_column_ - length};
        }
        return Token{type, std::string(input_.substr(start_pos, length)), line_, column_ - length};
    }

    Token Lexer::scan_string() {
        if (using_stream_) {
            size_t start_line = stream_line_;
            size_t start_column = stream_column_;
            advance(); // skip opening quote

            std::string value;
            bool escape = false;
            std::cout << "[scan_string] buffer before: ";
            for (char c : stream_buffer_)
                std::cout << "'" << c << "' ";
            std::cout << std::endl;

            while (true) {
                char ch = current_char();
                if (ch == '\0') {
                    // Unterminated string

                    return Token{token::Type::INVALID, value, stream_line_, stream_column_};
                }
                if (escape) {
                    switch (ch) {
                    case '"':
                        value += '"';
                        break;
                    case '\\':
                        value += '\\';
                        break;
                    case '/':
                        value += '/';
                        break;
                    case 'b':
                        value += '\b';
                        break;
                    case 'f':
                        value += '\f';
                        break;
                    case 'n':
                        value += '\n';
                        break;
                    case 'r':
                        value += '\r';
                        break;
                    case 't':
                        value += '\t';
                        break;
                    default:
                        value += ch;
                        break; // fallback: add raw char
                    }
                    escape = false;
                }
                else if (ch == '\\') {
                    escape = true;
                }
                else if (ch == '"') {
                    // End of string
                    break;
                }
                else {
                    value += ch;
                }
                advance();
            }
            advance(); // skip closing quote

            std::cout << "[scan_string] value='" << value << "'" << std::endl;
            std::cout << "[scan_string] buffer after: ";
            for (char c : stream_buffer_)
                std::cout << "'" << c << "' ";
            std::cout << std::endl;

            return Token{token::Type::STRING, value, start_line, start_column};
        }
        // Original string version
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
            return Token{token::Type::INVALID, std::string(input_.substr(quote_pos)), line_, start_column + 1};
        }

        size_t length = position_ - start_pos;
        std::string str_value = std::string(input_.substr(start_pos, length));

        advance();

        return Token{token::Type::STRING, str_value, line_, start_column};
    }

    Token Lexer::scan_number() {
        if (using_stream_) {
            // Capture starting position BEFORE reading the number
            size_t start_line = stream_line_;
            size_t start_column = stream_column_;
            std::string value;

            std::cout << "[scan_number] buffer before: ";
            for (char c : stream_buffer_)
                std::cout << "'" << c << "' ";
            std::cout << std::endl;

            auto is_number_char = [](char c) {
                return std::isdigit(c) || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E';
            };

            // Only consume number characters
            while (is_number_char(current_char())) {
                value += current_char();
                advance();
            }

            std::cout << "[scan_number] value='" << value << "'" << std::endl;
            std::cout << "[scan_number] buffer after: ";
            for (char c : stream_buffer_)
                std::cout << "'" << c << "' ";
            std::cout << std::endl;

            // Validate the number format
            try {
                std::stod(value);
            }
            catch (...) {

                return Token{token::Type::INVALID, value, start_line, start_column};
            }

            // Return the token with the correct value and starting position
            return Token{token::Type::NUMBER, value, start_line, start_column};
        }
        // Original string version
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
            return Token{token::Type::INVALID, std::string(input_.substr(start_pos)), line_, start_column};
        }

        if (current_char() == '.') {
            advance();
            if (!std::isdigit(current_char())) {
                return Token{token::Type::INVALID, std::string(input_.substr(start_pos)), line_, start_column};
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
                return Token{token::Type::INVALID, std::string(input_.substr(start_pos)), line_, start_column};
            }
            while (std::isdigit(current_char())) {
                advance();
            }
        }

        size_t length = position_ - start_pos;
        return Token{token::Type::NUMBER, std::string(input_.substr(start_pos, length)), line_, start_column};
    }

    Token Lexer::scan_keyword() {
        if (using_stream_) {
            size_t start_line = stream_line_;
            size_t start_column = stream_column_;
            std::string word;
            while (std::isalpha(current_char())) {
                word += current_char();
                advance();
            }
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
            return Token{type, word, start_line, start_column};
        }
        // Original string version
        const size_t start_pos = position_;
        const size_t start_column = column_;

        while (std::isalpha(current_char())) {
            advance();
        }

        size_t length = position_ - start_pos;
        std::string word = std::string(input_.substr(start_pos, length));

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

    Lexer::Lexer(std::string_view input) : input_(input), position_(0), line_(1), column_(1), using_stream_(false) {}

    Lexer::Lexer(std::istream& input) : input_stream_(&input), using_stream_(true), stream_line_(1), stream_column_(1) {
        // Fill buffer with first character for lookahead
        if (input_stream_ && input_stream_->good()) {
            int next = input_stream_->get();
            if (next != std::char_traits<char>::eof()) {
                stream_buffer_.push_back(static_cast<char>(next));
            }
        }
    }

    Token Lexer::next_token() {
        skip_whitespace();

        if (using_stream_) {
            if (current_char() == '\0') {
                std::cout << "[next_token] EOF at line " << stream_line_ << ", column " << stream_column_ << std::endl;
                std::cout << "[next_token] Buffer: ";
                for (char c : stream_buffer_)
                    std::cout << "'" << c << "' ";
                std::cout << std::endl;
                return Token{token::Type::EOF_TOKEN, "", stream_line_, stream_column_};
            }
        }
        else {
            if (position_ >= input_.size()) {
                return Token{token::Type::EOF_TOKEN, "", line_, column_};
            }
        }

        char ch = current_char();
        size_t current_column = using_stream_ ? stream_column_ : column_;

        if (using_stream_) {
            std::cout << "[next_token] Processing char '" << ch << "' (int: " << int(ch) << ") at line " << stream_line_
                      << ", column " << stream_column_ << std::endl;
            std::cout << "[next_token] Buffer: ";
            for (char c : stream_buffer_)
                std::cout << "'" << c << "' ";
            std::cout << std::endl;
        }

        Token token;
        switch (ch) {
        case '{':
            advance();
            if (using_stream_) {
                std::cout << "[next_token] Token: LBRACE" << std::endl;
                token = Token{token::Type::LBRACE, "{", stream_line_, current_column};
            }
            else {
                token = Token{token::Type::LBRACE, std::string(input_.substr(position_ - 1, 1)), line_, current_column};
            }
            break;
        case '}':
            advance();
            if (using_stream_) {
                std::cout << "[next_token] Token: RBRACE" << std::endl;
                token = Token{token::Type::RBRACE, "}", stream_line_, current_column};
            }
            else {
                token = Token{token::Type::RBRACE, std::string(input_.substr(position_ - 1, 1)), line_, current_column};
            }
            break;
        case '[':
            advance();
            if (using_stream_) {
                std::cout << "[next_token] Token: LBRACKET" << std::endl;
                token = Token{token::Type::LBRACKET, "[", stream_line_, current_column};
            }
            else {
                token =
                    Token{token::Type::LBRACKET, std::string(input_.substr(position_ - 1, 1)), line_, current_column};
            }
            break;
        case ']':
            advance();
            if (using_stream_) {
                std::cout << "[next_token] Token: RBRACKET" << std::endl;
                token = Token{token::Type::RBRACKET, "]", stream_line_, current_column};
            }
            else {
                token =
                    Token{token::Type::RBRACKET, std::string(input_.substr(position_ - 1, 1)), line_, current_column};
            }
            break;
        case ',':
            advance();
            if (using_stream_) {
                std::cout << "[next_token] Token: COMMA" << std::endl;
                token = Token{token::Type::COMMA, ",", stream_line_, current_column};
            }
            else {
                token = Token{token::Type::COMMA, std::string(input_.substr(position_ - 1, 1)), line_, current_column};
            }
            break;
        case ':':
            advance();
            if (using_stream_) {
                std::cout << "[next_token] Token: COLON" << std::endl;
                token = Token{token::Type::COLON, ":", stream_line_, current_column};
            }
            else {
                token = Token{token::Type::COLON, std::string(input_.substr(position_ - 1, 1)), line_, current_column};
            }
            break;
        case '"':
            if (using_stream_)
                std::cout << "[next_token] Token: STRING" << std::endl;
            token = scan_string();
            break;
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
            if (using_stream_)
                std::cout << "[next_token] Token: NUMBER" << std::endl;
            token = scan_number();
            break;
        default:
            if (std::isalpha(ch)) {
                if (using_stream_)
                    std::cout << "[next_token] Token: KEYWORD" << std::endl;
                token = scan_keyword();
            }
            else {
                advance();
                if (using_stream_) {
                    std::cout << "[next_token] Token: INVALID ('" << ch << "')" << std::endl;
                    token = Token{token::Type::INVALID, std::string(1, ch), stream_line_, current_column};
                }
                else {
                    token = Token{token::Type::INVALID, std::string(input_.substr(position_ - 1, 1)), line_,
                                  current_column};
                }
            }
            break;
        }
        if (using_stream_) {
            std::cout << "[next_token] Returned token: type=" << static_cast<int>(token.type_) << ", value='"
                      << token.value << "', line=" << token.line << ", column=" << token.column << std::endl;
            std::cout << "[next_token] Buffer after token: ";
            for (char c : stream_buffer_)
                std::cout << "'" << c << "' ";
            std::cout << std::endl;
        }
        return token;
    }

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        Token token;

        do {
            token = next_token();
            tokens.push_back(token);
        }
        while (token.type_ != token::Type::EOF_TOKEN && token.type_ != token::Type::INVALID);

        return tokens;
    }

} // namespace choochoo::json
