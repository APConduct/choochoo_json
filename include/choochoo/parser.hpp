#pragma once
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>
#include "choochoo/lexer.hpp"
#include "choochoo/token.hpp"
#include "choochoo/value.hpp"

namespace choochoo::json {
    struct Parser {
    private:
        std::reference_wrapper<Lexer> lexer_;
        Token current_token_;
        std::unordered_set<std::string> key_pool_; // For string interning of object keys

    public:
        Token current_token();
        void advance();
        std::expected<void, std::string> expect(token::Type expected);
        std::expected<std::string, std::string> process_string(std::string_view raw_string);
        double process_number(std::string_view number_str);

        std::expected<Value, std::string> parse_value();
        std::expected<Value, std::string> parse_object_body();
        std::expected<Value, std::string> parse_array_body();

        explicit Parser(Lexer& lexer);

        std::expected<Value, std::string> parse();
    };
} // namespace choochoo::json
