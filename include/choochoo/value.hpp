#pragma once
#include <functional>
#include <optional>
#include <sstream>
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
            // TODO: Fully implement move assignment operator
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


        [[nodiscard]] std::optional<double> as_number() const {
            if (type_ != Type::NUMBER) {
                return std::nullopt;
            }
            return storage_.number;
        };

        [[nodiscard]] std::optional<bool> as_boolean() const {
            if (type_ != Type::BOOLEAN) {
                return std::nullopt;
            }
            return storage_.boolean;
        }

        [[nodiscard]] std::optional<std::reference_wrapper<const std::string>> as_string() const {
            if (type_ != Type::STRING) {
                return std::nullopt;
            }
            return std::ref(storage_.string);
        }

        [[nodiscard]] std::optional<std::reference_wrapper<const std::vector<Value>>> as_array() {
            if (type_ != Type::ARRAY) {
                return std::nullopt;
            }
            return std::ref(storage_.array);
        }

        [[nodiscard]] std::optional<std::reference_wrapper<std::unordered_map<std::string, Value>>> as_object() {
            if (type_ != Type::OBJECT) {
                return std::nullopt;
            }
            return std::ref(storage_.object);
        }

        /// Pretty print the value as JSON
        std::stringstream pretty() {
            // TODO// implement pretty printing
        }
    };
} // namespace choochoo::json
