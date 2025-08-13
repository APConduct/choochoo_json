#pragma once
#include <expected>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include "choochoo/lexer.hpp"
#include "choochoo/token.hpp"
#include "choochoo/value.hpp"

namespace choochoo::json {
    struct Parser {
    private:
        std::reference_wrapper<Lexer> lexer_;
        Token current_token_;

    public:
        Token current_token() { return current_token_; }

        void advance() { current_token_ = lexer_.get().next_token(); }

        std::expected<void, std::string> expect(token::Type expected) {
            if (current_token_.type_ != expected) {
                return std::unexpected("Unexpected token at line " + std::to_string(current_token_.line) + ", column " +
                                       std::to_string(current_token_.column) +
                                       ". Expected: " + std::to_string(static_cast<int>(expected)) +
                                       ", got: " + std::to_string(static_cast<int>(current_token_.type_)));
            }
            advance();
            return {};
        }

        std::string process_string(std::string_view raw_string) {
            std::string result;
            result.reserve(raw_string.size());

            for (size_t i = 0; i < raw_string.size(); ++i) {
                if (raw_string[i] == '\\' && i + 1 < raw_string.size()) {
                    switch (raw_string[i + 1]) {
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case '/':
                        result += '/';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    default:
                        throw std::runtime_error("Invalid escape sequence");
                    }
                    ++i;
                }
                else {
                    result += raw_string[i];
                }
            }
            return result;
        }

        double process_number(std::string_view number_str) {
            try {
                // Convert std::string_view to std::string for stod
                return std::stod(std::string(number_str));
            }
            catch (const std::exception&) {
                throw std::runtime_error("Invalid number format");
            }
        }

        std::expected<Value, std::string> parse_value() {
            if (current_token_.type_ == token::Type::EOF_TOKEN) {
                return std::unexpected("No value to parse (unexpected EOF)");
            }
            switch (current_token_.type_) {
            case token::Type::STRING: {
                std::string processed = process_string(current_token_.value);
                advance();
                return Value::string(std::move(processed));
            }
            case token::Type::NUMBER: {
                double num;
                try {
                    num = process_number(current_token_.value);
                }
                catch (const std::exception&) {
                    return std::unexpected("Invalid number format");
                }
                advance();
                return Value::number(num);
            }
            case token::Type::TRUE:
                advance();
                return Value::boolean(true);
            case token::Type::FALSE:
                advance();
                return Value::boolean(false);
            case token::Type::NULL_VALUE:
                advance();
                return Value::null();
            case token::Type::LBRACE: {
                auto expect_result = expect(token::Type::LBRACE);
                if (!expect_result)
                    return std::unexpected(expect_result.error());
                auto obj_result = parse_object_body();
                if (!obj_result)
                    return std::unexpected(obj_result.error());
                return obj_result;
            }
            case token::Type::LBRACKET: {
                auto expect_result = expect(token::Type::LBRACKET);
                if (!expect_result)
                    return std::unexpected(expect_result.error());
                auto arr_result = parse_array_body();
                if (!arr_result)
                    return std::unexpected(arr_result.error());
                return arr_result;
            }
            default: {
                std::string err =
                    "Unexpected token in parse_value: type=" + std::to_string(static_cast<int>(current_token_.type_)) +
                    ", value='" + std::string(current_token_.value) + "'";
                return std::unexpected(err);
            }
            }
        }
        std::expected<Value, std::string> parse_object_body() {
            std::unordered_map<std::string, Value> obj;
            if (current_token_.type_ == token::Type::RBRACE) {
                advance();
                return Value::object(std::move(obj));
            }
            while (true) {
                if (current_token_.type_ != token::Type::STRING) {
                    return std::unexpected("Expected string key in object");
                }
                std::string key = process_string(current_token_.value);
                advance();
                auto expect_result = expect(token::Type::COLON);
                if (!expect_result)
                    return std::unexpected(expect_result.error());

                auto value_result = parse_value();
                if (!value_result)
                    return std::unexpected(value_result.error());
                obj[std::move(key)] = std::move(value_result.value());

                if (current_token_.type_ == token::Type::COMMA) {
                    advance();
                    continue;
                }
                else if (current_token_.type_ == token::Type::RBRACE) {
                    advance();
                    break;
                }
                else {
                    return std::unexpected("Expected ',' or '}' in object");
                }
            }
            return Value::object(std::move(obj));
        }

        std::expected<Value, std::string> parse_array_body() {
            std::vector<Value> arr;

            if (current_token_.type_ == token::Type::RBRACKET) {
                advance();
                return Value::array(std::move(arr));
            }

            while (true) {
                auto value_result = parse_value();
                if (!value_result)
                    return std::unexpected(value_result.error());
                arr.push_back(std::move(value_result.value()));

                if (current_token_.type_ == token::Type::COMMA) {
                    advance();
                    continue;
                }
                else if (current_token_.type_ == token::Type::RBRACKET) {
                    advance();
                    break;
                }
                else {
                    return std::unexpected("Expected ',' or ']' in array");
                }
            }
            return Value::array(std::move(arr));
        }

    public:
        explicit Parser(Lexer& lexer) : lexer_(lexer) { advance(); }

        std::expected<Value, std::string> parse() {
            auto result = parse_value();
            if (!result) {
                return std::unexpected(result.error());
            }
            if (current_token_.type_ != token::Type::EOF_TOKEN) {
                return std::unexpected("Unexpected content after JSON value");
            }
            return result;
        }
    };
} // namespace choochoo::json
