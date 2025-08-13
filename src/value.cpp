#include "choochoo/value.hpp"
#include <cstdio>
#include <sstream>

namespace choochoo::json {

    Value::Value() : type_(Type::NULL_VALUE) {}

    Value::~Value() {
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

    Value::Value(const Value& other) : type_(other.type_) {
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

    Value::Value(Value&& other) noexcept : type_(other.type_) {
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

    Value& Value::operator=(const Value& other) {
        if (this != &other) {
            this->~Value();
            new (this) Value(other);
        }
        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept {
        if (this != &other) {
            this->~Value();
            new (this) Value(std::move(other));
        }
        return *this;
    }

    Value Value::null() { return {}; }

    Value Value::boolean(const bool b) {
        Value v;
        v.type_ = Type::BOOLEAN;
        v.storage_.boolean = b;
        return v;
    }

    Value Value::number(const double n) {
        Value v;
        v.type_ = Type::NUMBER;
        v.storage_.number = n;
        return v;
    }

    Value Value::string(std::string s) {
        Value v;
        v.type_ = Type::STRING;
        new (&v.storage_.string) std::string(std::move(s));
        return v;
    }

    Value Value::array(std::vector<Value> arr) {
        Value v;
        v.type_ = Type::ARRAY;
        new (&v.storage_.array) std::vector(std::move(arr));
        return v;
    }

    Value Value::object(std::unordered_map<std::string, Value> obj) {
        Value v;
        v.type_ = Type::OBJECT;
        new (&v.storage_.object) std::unordered_map(std::move(obj));
        return v;
    }

    Type Value::type() const { return type_; }

    std::optional<double> Value::as_number() const {
        if (type_ != Type::NUMBER) {
            return std::nullopt;
        }
        return storage_.number;
    }

    std::optional<bool> Value::as_boolean() const {
        if (type_ != Type::BOOLEAN) {
            return std::nullopt;
        }
        return storage_.boolean;
    }

    std::optional<std::reference_wrapper<const std::string>> Value::as_string() const {
        if (type_ != Type::STRING) {
            return std::nullopt;
        }
        return std::ref(storage_.string);
    }

    std::optional<std::reference_wrapper<const std::vector<Value>>> Value::as_array() {
        if (type_ != Type::ARRAY) {
            return std::nullopt;
        }
        return std::ref(storage_.array);
    }

    std::optional<std::reference_wrapper<const std::vector<Value>>> Value::as_array() const {
        if (type_ != Type::ARRAY) {
            return std::nullopt;
        }
        return std::cref(storage_.array);
    }

    std::optional<std::reference_wrapper<std::unordered_map<std::string, Value>>> Value::as_object() {
        if (type_ != Type::OBJECT) {
            return std::nullopt;
        }
        return std::ref(storage_.object);
    }

    std::optional<std::reference_wrapper<const std::unordered_map<std::string, Value>>> Value::as_object() const {
        if (type_ != Type::OBJECT) {
            return std::nullopt;
        }
        return std::cref(storage_.object);
    }

    std::string Value::pretty(int indent) const {
        const std::string indent_str(indent, ' ');
        const std::string indent_next(indent + 2, ' ');

        switch (type_) {
        case Type::NULL_VALUE:
            return "null";
        case Type::BOOLEAN:
            return storage_.boolean ? "true" : "false";
        case Type::NUMBER: {
            std::ostringstream oss;
            oss << storage_.number;
            return oss.str();
        }
        case Type::STRING: {
            std::string out = "\"";
            for (char c : storage_.string) {
                switch (c) {
                case '\"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\b':
                    out += "\\b";
                    break;
                case '\f':
                    out += "\\f";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        out += "\\u";
                        char buf[5];
                        snprintf(buf, sizeof(buf), "%04x", c);
                        out += buf;
                    }
                    else {
                        out += c;
                    }
                }
            }
            out += "\"";
            return out;
        }
        case Type::ARRAY: {
            const auto& arr = storage_.array;
            if (arr.empty())
                return "[]";
            std::string out = "[\n";
            for (size_t i = 0; i < arr.size(); ++i) {
                out += indent_next + arr[i].pretty(indent + 2);
                if (i + 1 < arr.size())
                    out += ",";
                out += "\n";
            }
            out += indent_str + "]";
            return out;
        }
        case Type::OBJECT: {
            const auto& obj = storage_.object;
            if (obj.empty())
                return "{}";
            std::string out = "{\n";
            size_t i = 0;
            for (const auto& [key, value] : obj) {
                out += indent_next + "\"" + key + "\": " + value.pretty(indent + 2);
                if (i + 1 < obj.size())
                    out += ",";
                out += "\n";
                ++i;
            }
            out += indent_str + "}";
            return out;
        }
        default:
            return "";
        }
    }

    // --- Iterator support ---

    // Array iterators
    std::vector<Value>::iterator Value::begin() {
        if (type_ != Type::ARRAY)
            throw std::logic_error("Value is not an array");
        return storage_.array.begin();
    }
    std::vector<Value>::iterator Value::end() {
        if (type_ != Type::ARRAY)
            throw std::logic_error("Value is not an array");
        return storage_.array.end();
    }
    std::vector<Value>::const_iterator Value::begin() const {
        if (type_ != Type::ARRAY)
            throw std::logic_error("Value is not an array");
        return storage_.array.begin();
    }
    std::vector<Value>::const_iterator Value::end() const {
        if (type_ != Type::ARRAY)
            throw std::logic_error("Value is not an array");
        return storage_.array.end();
    }

    // Object iterators
    std::unordered_map<std::string, Value>::iterator Value::obj_begin() {
        if (type_ != Type::OBJECT)
            throw std::logic_error("Value is not an object");
        return storage_.object.begin();
    }
    std::unordered_map<std::string, Value>::iterator Value::obj_end() {
        if (type_ != Type::OBJECT)
            throw std::logic_error("Value is not an object");
        return storage_.object.end();
    }
    std::unordered_map<std::string, Value>::const_iterator Value::obj_begin() const {
        if (type_ != Type::OBJECT)
            throw std::logic_error("Value is not an object");
        return storage_.object.begin();
    }
    std::unordered_map<std::string, Value>::const_iterator Value::obj_end() const {
        if (type_ != Type::OBJECT)
            throw std::logic_error("Value is not an object");
        return storage_.object.end();
    }

} // namespace choochoo::json
