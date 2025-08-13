#include <choochoo/json.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

// Helper to find interned key pointer in object map
const std::string* find_key(const std::unordered_map<const std::string*, choochoo::json::Value>& obj,
                            const std::string& key) {
    for (const auto& [kptr, _] : obj) {
        if (*kptr == key)
            return kptr;
    }
    return nullptr;
}

int main() {
    std::string json_input = R"({
        "numbers": [1, 2, 3, 4, 5],
        "person": {
            "name": "Bob",
            "age": 42,
            "languages": ["C++", "Python", "Rust"]
        }
    })";

    choochoo::json::Lexer lexer(json_input);
    choochoo::json::Parser parser(lexer);

    auto result = parser.parse();
    if (!result) {
        std::cerr << "Parse error: " << result.error() << std::endl;
        return 1;
    }
    auto root = result.value();

    // --- Array iteration example ---
    std::cout << "Iterating over 'numbers' array using range-based for:\n";
    auto obj_opt = root.as_object();
    const std::string* numbers_kptr = obj_opt ? find_key(obj_opt->get(), "numbers") : nullptr;
    if (obj_opt && numbers_kptr) {
        const auto& numbers_val = obj_opt->get().at(numbers_kptr);
        if (numbers_val.type() == choochoo::json::Type::ARRAY) {
            for (const auto& num : numbers_val) {
                auto n = num.as_number();
                if (n) {
                    std::cout << *n << " ";
                }
            }
            std::cout << std::endl;
        }
    }

    // --- Array iteration using manual iterators ---
    std::cout << "Iterating over 'languages' array using manual iterators:\n";
    const std::string* person_kptr = obj_opt ? find_key(obj_opt->get(), "person") : nullptr;
    if (obj_opt && person_kptr) {
        const auto& person_val = obj_opt->get().at(person_kptr);
        auto person_obj = person_val.as_object();
        const std::string* langs_kptr = person_obj ? find_key(person_obj->get(), "languages") : nullptr;
        if (person_obj && langs_kptr) {
            const auto& langs_val = person_obj->get().at(langs_kptr);
            if (langs_val.type() == choochoo::json::Type::ARRAY) {
                for (auto it = langs_val.begin(); it != langs_val.end(); ++it) {
                    auto str = it->as_string();
                    if (str) {
                        std::cout << str->get() << " ";
                    }
                }
                std::cout << std::endl;
            }
        }
    }

    // --- Object iteration example ---
    std::cout << "Iterating over 'person' object using obj_begin/obj_end:\n";
    if (obj_opt && person_kptr) {
        const auto& person_val = obj_opt->get().at(person_kptr);
        if (person_val.type() == choochoo::json::Type::OBJECT) {
            for (auto it = person_val.obj_begin(); it != person_val.obj_end(); ++it) {
                std::cout << "  " << *(it->first) << ": ";
                if (it->second.type() == choochoo::json::Type::STRING) {
                    std::cout << it->second.as_string()->get();
                }
                else if (it->second.type() == choochoo::json::Type::NUMBER) {
                    std::cout << it->second.as_number().value();
                }
                else if (it->second.type() == choochoo::json::Type::ARRAY) {
                    std::cout << "[array]";
                }
                else {
                    std::cout << "[other]";
                }
                std::cout << std::endl;
            }
        }
    }

    // --- STL algorithm compatibility example ---
    std::cout << "Using std::find_if to search for number 3 in 'numbers':\n";
    if (obj_opt && numbers_kptr) {
        const auto& numbers_val = obj_opt->get().at(numbers_kptr);
        if (numbers_val.type() == choochoo::json::Type::ARRAY) {
            auto it = std::find_if(numbers_val.begin(), numbers_val.end(), [](const choochoo::json::Value& v) {
                auto n = v.as_number();
                return n && *n == 3;
            });
            if (it != numbers_val.end()) {
                std::cout << "Found number 3!" << std::endl;
            }
            else {
                std::cout << "Number 3 not found." << std::endl;
            }
        }
    }

    // --- Error handling: calling begin() on wrong type ---
    std::cout << "Demonstrating error handling for begin() on wrong type:\n";
    try {
        root.begin(); // root is an object, not an array
    }
    catch (const std::logic_error& e) {
        std::cout << "Caught exception as expected: " << e.what() << std::endl;
    }

    try {
        root.obj_begin(); // root is an object, this should work
        std::cout << "obj_begin() on object succeeded." << std::endl;
    }
    catch (const std::logic_error& e) {
        std::cout << "Unexpected exception: " << e.what() << std::endl;
    }

    return 0;
}
