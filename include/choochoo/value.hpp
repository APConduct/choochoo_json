#pragma once
#include <expected>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace choochoo::json {
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
} // namespace choochoo::json
