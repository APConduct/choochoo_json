#pragma once
#include <functional>
#include <optional>
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
        Value();
        ~Value();
        Value(const Value& other);
        Value(Value&& other) noexcept;
        Value& operator=(const Value& other);
        Value& operator=(Value&& other) noexcept;

        static Value null();
        static Value boolean(const bool b);
        static Value number(const double n);
        static Value string(std::string s = "");
        static Value array(std::vector<Value> arr = {});
        static Value object(std::unordered_map<std::string, Value> obj = {});

        [[nodiscard]] Type type() const;

        [[nodiscard]] std::optional<double> as_number() const;
        [[nodiscard]] std::optional<bool> as_boolean() const;
        [[nodiscard]] std::optional<std::reference_wrapper<const std::string>> as_string() const;
        [[nodiscard]] std::optional<std::reference_wrapper<const std::vector<Value>>> as_array();
        [[nodiscard]] std::optional<std::reference_wrapper<std::unordered_map<std::string, Value>>> as_object();

        // Const-qualified overloads for read-only access
        [[nodiscard]] std::optional<std::reference_wrapper<const std::vector<Value>>> as_array() const;
        [[nodiscard]] std::optional<std::reference_wrapper<const std::unordered_map<std::string, Value>>>
        as_object() const;

        /// Pretty print the value as JSON
        std::string pretty(int indent = 0) const;

        // --- Iterator support ---

        // Array iterators
        std::vector<Value>::iterator begin();
        std::vector<Value>::iterator end();
        std::vector<Value>::const_iterator begin() const;
        std::vector<Value>::const_iterator end() const;

        // Object iterators
        std::unordered_map<std::string, Value>::iterator obj_begin();
        std::unordered_map<std::string, Value>::iterator obj_end();
        std::unordered_map<std::string, Value>::const_iterator obj_begin() const;
        std::unordered_map<std::string, Value>::const_iterator obj_end() const;
    };
} // namespace choochoo::json
