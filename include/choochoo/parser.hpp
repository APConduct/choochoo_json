#pragma once
#include <functional>
#include <iostream>
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

        void expect(token::Type expected) {
            if (current_token_.type_ != expected) {
                throw std::runtime_error("Unexpected tocken at line " + std::to_string(current_token_.line) +
                                         "' column " + std::to_string(current_token_.column));
            }
            advance();
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

        Value parse_value() {
            if (current_token_.type_ == token::Type::EOF_TOKEN) {
                throw std::runtime_error("No value to parse (unexpected EOF)");
            }
            switch (current_token_.type_) {
            case token::Type::STRING: {
                std::string processed = process_string(current_token_.value);
                advance();
                return Value::string(std::move(processed));
            }
            case token::Type::NUMBER: {
                double num = process_number(current_token_.value);
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
            case token::Type::LBRACE:
                return parse_object();
            case token::Type::LBRACKET:
                return parse_array();
            default:
                std::cerr << "DEBUG: Unexpected token in parse_value\n";
                std::cerr << "Token type: " << static_cast<int>(current_token_.type_) << ", value: '"
                          << current_token_.value << "', line: " << current_token_.line
                          << ", column: " << current_token_.column << std::endl;
                throw std::runtime_error("Unexpected token in parse_value");
            }
        }
        Value parse_object() {
            expect(token::Type::LBRACE);
            std::unordered_map<std::string, Value> obj;
            if (current_token_.type_ == token::Type::RBRACE) {
                advance();
                return Value::object(std::move(obj));
            }
            while (true) {
                if (current_token_.type_ != token::Type::STRING) {
                    throw std::runtime_error("Expected string key in object");
                }
                std::string key = process_string(current_token_.value);
                advance();
                expect(token::Type::COLON);
                Value value = parse_value();
                obj[std::move(key)] = std::move(value);

                if (current_token_.type_ == token::Type::COMMA) {
                    advance();
                    continue;
                }
                else if (current_token_.type_ == token::Type::RBRACE) {
                    advance();
                    break;
                }
                else {
                    throw std::runtime_error("Expected ',' or '}' in object");
                }
            }
            return Value::object(std::move(obj));
        }

        Value parse_array() {
            expect(token::Type::LBRACKET);
            std::vector<Value> arr;

            if (current_token_.type_ == token::Type::RBRACKET) {
                advance();
                return Value::array(std::move(arr));
            }

            while (true) {
                Value value = parse_value();
                arr.push_back(std::move(value));

                if (current_token_.type_ == token::Type::COMMA) {
                    advance();
                    continue;
                }
                else if (current_token_.type_ == token::Type::RBRACKET) {
                    advance();
                    break;
                }
                else {
                    throw std::runtime_error("Expected ',' or ']' in array");
                }
            }
            return Value::array(std::move(arr));
        }

    public:
        explicit Parser(Lexer& lexer) : lexer_(lexer) { advance(); }

        Value parse() {
            Value result = parse_value();

            if (current_token_.type_ != token::Type::EOF_TOKEN) {
                throw std::runtime_error("Unexpected content after JSON value");
            }
            return result;
        }
    };
} // namespace choochoo::json
