
#include <cctype>
#include <cstddef>
#include <exception>
#include <expected>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

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

    enum class Type { NULL_VALUE, BOOLEAN, NUMBER, STRING, ARRAY, OBJECT };

    struct Value {
    protected:
        Type type_{};

        union Storage {
            bool boolean{};
            double number;
            std::string string;
            std::unordered_map<std::string, Value> object;
            std::vector<Value> array;

            Storage() {}
            ~Storage() {}
        } storage_{};

    public:
        Value() : type_(Type::NULL_VALUE) {}

        ~Value() {
            switch (type_) {
            case Type::STRING:
                storage_.string.~basic_string();
                break;
            case Type::OBJECT:
                storage_.object.~unordered_map();
                break;
            case Type::ARRAY:
                storage_.array.~vector();
                break;
            case Type::BOOLEAN:
            case Type::NUMBER:
            default:
                break;
            }
        }

        Value(const Value& other) : type_(other.type_) {
            switch (type_) {
            case Type::BOOLEAN:
                storage_.boolean = other.storage_.boolean;
                break;
            case Type::NUMBER:
                storage_.number = other.storage_.number;
                break;
            case Type::STRING:
                new (&storage_.string) std::string(other.storage_.string);
                break;
            case Type::ARRAY:
                new (&storage_.array) std::vector(other.storage_.array);
                break;
            case Type::OBJECT:
                new (&storage_.object) std::unordered_map(other.storage_.object);
                break;
            case Type::NULL_VALUE:
                break;
            }
        }

        Value(Value&& other) noexcept : type_(other.type_) {
            switch (type_) {
            case Type::STRING:
                new (&storage_.string) std::string(std::move(other.storage_.string));
                break;
            case Type::ARRAY:
                new (&storage_.array) std::vector(std::move(other.storage_.array));
                break;
            case Type::OBJECT:
                new (&storage_.object) std::unordered_map(std::move(other.storage_.object));
                break;
            case Type::BOOLEAN:
                storage_.boolean = other.storage_.boolean;
                break;
            case Type::NUMBER:
                storage_.number = other.storage_.number;
                break;
            case Type::NULL_VALUE:
            default:
                break;
            }
            other.type_ = Type::NULL_VALUE;
        }

        Value& operator=(const Value& other) {
            // TODO: Fully implement assignment operator
            if (this != &other) {
                this->~Value();
                new (this) Value(other);
            }
            return *this;
        }

        Value& operator=(Value&& other) noexcept {
            if (this != &other) {
                this->~Value();
                new (this) Value(std::move(other));
            }
            return *this;
        }

        static Value null() { return {}; }

        static Value boolean(const bool b) {
            Value v;
            v.type_ = Type::BOOLEAN;
            v.storage_.boolean = b;
            return v;
        }

        static Value number(const double n) {
            Value v;
            v.type_ = Type::NUMBER;
            v.storage_.number = n;
            return v;
        }

        static Value string(std::string s = "") {
            Value v;
            v.type_ = Type::STRING;
            new (&v.storage_.string) std::string(std::move(s));
            return v;
        }

        static Value array(std::vector<Value> arr = {}) {
            Value v;
            v.type_ = Type::ARRAY;
            new (&v.storage_.array) std::vector(std::move(arr));
            return v;
        }

        static Value object(std::unordered_map<std::string, Value> obj = {}) {
            Value v;
            v.type_ = Type::OBJECT;
            new (&v.storage_.object) std::unordered_map(std::move(obj));
            return v;
        }

        [[nodiscard]] Type type() const { return type_; }

        enum TypeErrorType {
            NOT_A_BOOLEAN_ERROR,
            NOT_A_NUMBER_ERROR,
            NOT_A_STRING_ERROR,
            NOT_AN_ARRAY_ERROR,
            NOT_AN_OBJECT_ERROR,
            NOT_NULL_ERROR
        };

        using TypeError = std::pair<TypeErrorType, std::string>;

        [[nodiscard]] std::expected<double, TypeError> as_number() const {
            if (type_ != Type::NUMBER) {
                return std::unexpected<TypeError>(TypeError(NOT_A_NUMBER_ERROR, "VALUE IS NOT A NUMBER"));
            }
            return storage_.number;
        };

        [[nodiscard]] std::expected<bool, TypeError> as_boolean() const {
            if (type_ != Type::BOOLEAN) {
                return std::unexpected<TypeError>(TypeError(NOT_A_BOOLEAN_ERROR, "VALUE IS NOT A BOOLEAN"));
            }
            return storage_.boolean;
        }

        [[nodiscard]] std::expected<std::reference_wrapper<const std::string>, TypeError> as_string() const {
            if (type_ != Type::STRING) {
                return std::unexpected<TypeError>(TypeError(NOT_A_STRING_ERROR, "VALUE IS NOT A STRING"));
            }
            return std::ref(storage_.string);
        }

        [[nodiscard]] std::expected<std::reference_wrapper<const std::vector<Value>>, TypeError> as_array() {
            if (type_ != Type::ARRAY) {
                return std::unexpected<TypeError>(TypeError(NOT_AN_ARRAY_ERROR, "VALUE IS A NOT AN ARRAY"));
            }
            return std::ref(storage_.array);
        }

        [[nodiscard]] std::expected<std::reference_wrapper<std::unordered_map<std::string, Value>>, TypeError>
        as_object() {
            if (type_ != Type::OBJECT) {
                return std::unexpected<TypeError>(TypeError(NOT_AN_OBJECT_ERROR, "VALUE IS NOT AN OBJECT"));
            }
            return std::ref(storage_.object);
        }
    };

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
            if (raw_string.size() < 2) {
                throw std::runtime_error("Invalid string token");
            }
            std::string_view content = raw_string.substr(1, raw_string.size() - 2);
            std::string result;
            result.reserve(content.size());

            for (size_t i = 0; i < content.size(); ++i) {
                if (content[i] == '\\' && i + 1 < content.size()) {
                    switch (content[i + 1]) {
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
                    result += content[i];
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
                // Gracefully handle EOF at top-level
                return Value::null();
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

int main() {
    std::string json_input = R"({
            "name": "John",
            "age": 30,
            "active": true,
            "scores": [95, 87, 92],
            "metadata": {"created": "2024", "version": 1.2}
        })";

    try {
        choochoo::json::Lexer lexer(json_input);
        auto tokens = lexer.tokenize();

        std::cout << "Input received:" << "\n" << json_input << '\n' << '\n' << "Tokens:" << '\n';

        for (const auto token : tokens) {
            std::cout << "Type: " << static_cast<int>(token.type_) << ", Value: '" << token.value << "'"
                      << ", Line: " << token.line << ", Column: " << token.column << '\n';
        }

        choochoo::json::Parser parser(lexer);

        auto root = parser.parse();

        // Check for EOF after parsing root value
        if (parser.current_token().type_ != choochoo::json::token::Type::EOF_TOKEN) {
            throw std::runtime_error("Extra data after root JSON value");
        }

        std::cout << "Successfully parsed JSON!" << std::endl;
        std::cout << "Root type: " << static_cast<int>(root.type()) << std::endl;

        // Example access
        if (root.type() == choochoo::json::Type::OBJECT) {
            const auto& obj = root.as_object();
            if (obj->get().find("name") != obj->get().end()) {
                std::cout << "Name: " << obj->get().at("name").as_string()->get() << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Parser error: " << e.what() << '\n';
    }
}
