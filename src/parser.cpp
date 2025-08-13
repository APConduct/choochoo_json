#include "choochoo/parser.hpp"
#include <iostream>
#include <sstream>

namespace choochoo::json {

    const char* token_type_name(token::Type type) {
        switch (type) {
        case token::Type::STRING:
            return "STRING";
        case token::Type::NUMBER:
            return "NUMBER";
        case token::Type::TRUE:
            return "TRUE";
        case token::Type::FALSE:
            return "FALSE";
        case token::Type::NULL_VALUE:
            return "NULL";
        case token::Type::LBRACE:
            return "LBRACE";
        case token::Type::RBRACE:
            return "RBRACE";
        case token::Type::LBRACKET:
            return "LBRACKET";
        case token::Type::RBRACKET:
            return "RBRACKET";
        case token::Type::COMMA:
            return "COMMA";
        case token::Type::COLON:
            return "COLON";
        case token::Type::EOF_TOKEN:
            return "EOF";
        case token::Type::INVALID:
            return "INVALID";
        default:
            return "UNKNOWN";
        }
    }

    Token Parser::current_token() { return current_token_; }

    void Parser::advance() { current_token_ = lexer_.get().next_token(); }

    std::expected<void, std::string> Parser::expect(token::Type expected) {
        if (current_token_.type_ != expected) {
            return std::unexpected("Unexpected token at line " + std::to_string(current_token_.line) + ", column " +
                                   std::to_string(current_token_.column) +
                                   ". Expected: " + std::to_string(static_cast<int>(expected)) +
                                   ", got: " + std::to_string(static_cast<int>(current_token_.type_)));
        }
        advance();
        return {};
    }

    std::expected<std::string, std::string> Parser::process_string(std::string_view raw_string) {
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
                    return std::unexpected("Invalid escape sequence");
                }
                ++i;
            }
            else {
                result += raw_string[i];
            }
        }
        return result;
    }


    // Helper to extract string from Token.value variant
    static std::string_view token_string_view(const Token& token) {
        if (std::holds_alternative<std::string_view>(token.value)) {
            return std::get<std::string_view>(token.value);
        }
        else {
            return std::get<std::string>(token.value);
        }
    }

    double Parser::process_number(std::string_view number_str) {
        try {
            return std::stod(std::string(number_str));
        }
        catch (const std::exception&) {
            throw std::runtime_error("Invalid number format");
        }
    }

    std::expected<Value, std::string> Parser::parse_value() {
        // std::cout << "[parse_value] current_token_: type=" << token_type_name(current_token_.type_) << ", value='"
        //           << current_token_.value << "', line=" << current_token_.line << ", column=" <<
        //           current_token_.column
        //           << std::endl;
        if (current_token_.type_ == token::Type::EOF_TOKEN) {
            return std::unexpected("No value to parse (unexpected EOF)");
        }
        switch (current_token_.type_) {
        case token::Type::STRING: {
            auto processed_result = process_string(token_string_view(current_token_));
            if (!processed_result)
                return std::unexpected(processed_result.error());
            std::string processed = std::move(processed_result.value());
            advance();
            return Value::string(std::move(processed));
        }
        case token::Type::NUMBER: {
            double num;
            try {
                num = process_number(token_string_view(current_token_));
            }
            catch (const std::exception&) {
                std::ostringstream oss;
                oss << "Invalid number format at line " << current_token_.line << ", column " << current_token_.column
                    << ". Value: '" << token_string_view(current_token_) << "'";
                return std::unexpected(oss.str());
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
            std::ostringstream oss;
            oss << "Unexpected token '" << token_string_view(current_token_) << "' ("
                << token_type_name(current_token_.type_) << ") at line " << current_token_.line << ", column "
                << current_token_.column << ".";
            return std::unexpected(oss.str());
        }
        }
    }

    std::expected<Value, std::string> Parser::parse_object_body() {
        std::unordered_map<std::string, Value> obj;
        obj.reserve(8); // TODO: Profile typical object sizes and adjust reservation for optimal performance.
        if (current_token_.type_ == token::Type::RBRACE) {
            advance();
            return Value::object(std::move(obj));
        }
        while (true) {
            if (current_token_.type_ != token::Type::STRING) {
                std::ostringstream oss;
                oss << "Expected string key in object, but found '" << token_string_view(current_token_) << "' ("
                    << token_type_name(current_token_.type_) << ") at line " << current_token_.line << ", column "
                    << current_token_.column << ".";
                return std::unexpected(oss.str());
            }
            auto key_result = process_string(token_string_view(current_token_));
            if (!key_result)
                return std::unexpected(key_result.error());
            std::string key = std::move(key_result.value());
            advance();
            auto expect_result = expect(token::Type::COLON);
            if (!expect_result)
                return std::unexpected(expect_result.error());

            auto value_result = parse_value();
            if (!value_result)
                return std::unexpected(value_result.error());
            obj.emplace(std::move(key), std::move(value_result.value()));

            if (current_token_.type_ == token::Type::COMMA) {
                advance();
                continue;
            }
            else if (current_token_.type_ == token::Type::RBRACE) {
                advance();
                break;
            }
            else {
                std::ostringstream oss;
                oss << "Expected ',' or '}' in object, but found '" << token_string_view(current_token_) << "' ("
                    << token_type_name(current_token_.type_) << ") at line " << current_token_.line << ", column "
                    << current_token_.column << ".";
                return std::unexpected(oss.str());
            }
        }
        return Value::object(std::move(obj));
    }

    std::expected<Value, std::string> Parser::parse_array_body() {
        std::vector<Value> arr;
        arr.reserve(8); // TODO: Profile typical array sizes and adjust reservation for optimal performance.

        if (current_token_.type_ == token::Type::RBRACKET) {
            advance();
            return Value::array(std::move(arr));
        }

        while (true) {
            auto value_result = parse_value();
            if (!value_result)
                return std::unexpected(value_result.error());
            arr.emplace_back(std::move(value_result.value()));

            if (current_token_.type_ == token::Type::COMMA) {
                advance();
                continue;
            }
            else if (current_token_.type_ == token::Type::RBRACKET) {
                advance();
                break;
            }
            else {
                std::ostringstream oss;
                oss << "Expected ',' or ']' in array, but found '" << token_string_view(current_token_) << "' ("
                    << token_type_name(current_token_.type_) << ") at line " << current_token_.line << ", column "
                    << current_token_.column << ".";
                return std::unexpected(oss.str());
            }
        }
        return Value::array(std::move(arr));
    }

    Parser::Parser(Lexer& lexer) : lexer_(lexer) { advance(); }

    std::expected<Value, std::string> Parser::parse() {
        auto result = parse_value();
        if (!result) {
            return std::unexpected(result.error());
        }
        if (current_token_.type_ != token::Type::EOF_TOKEN) {
            return std::unexpected("Unexpected content after JSON value");
        }
        return result;
    }

} // namespace choochoo::json
