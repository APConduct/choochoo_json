#pragma once
#include <cctype>
#include <istream>
#include <string_view>
#include <vector>
#include "choochoo/token.hpp"

namespace choochoo::json {
    struct Lexer {
    protected:
        // For string input
        std::string_view input_;
        size_t position_{}, line_{}, column_{};

        // For stream input
        std::istream* input_stream_{nullptr};
        static constexpr size_t STREAM_BUFFER_SIZE = 8;
        char stream_buffer_[STREAM_BUFFER_SIZE]{};
        size_t stream_head_{0}, stream_tail_{0}, stream_count_{0};
        bool using_stream_{false};
        size_t stream_line_{1}, stream_column_{1};

        [[nodiscard]] char current_char() const;
        [[nodiscard]] char peek_char(size_t offset = 1);
        void advance();
        void skip_whitespace();
        [[nodiscard]] Token make_token(token::Type type, size_t start_pos, size_t length) const;
        Token scan_string();
        Token scan_number();
        Token scan_keyword();

    public:
        explicit Lexer(std::string_view input);
        explicit Lexer(std::istream& input);

        Token next_token();
        std::vector<Token> tokenize();
    };
} // namespace choochoo::json
