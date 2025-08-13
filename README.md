# ChooChoo JSON

A modern, performant C++ JSON parser library that just keeps chuggin (like high speed rail).

## Features

- Fast, standards-compliant JSON parsing
- Modern C++23 error handling with `std::expected`
- Easy to use API
- Example and test suite included

## Build & Test

```sh
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S . -B ./cmake-build-debug
ninja -C ./cmake-build-debug
./cmake-build-debug/choochoo_json_tests
```

## Usage Example

See `examples/basic_usage.cpp` for a full example.

**Recommended: Include the facade header for all core functionality:**

```cpp
#include <choochoo/json.hpp>

std::string json = R"({"hello": "world"})";
choochoo::json::Lexer lexer(json);
choochoo::json::Parser parser(lexer);
auto result = parser.parse();
if (result) {
    std::cout << result.value().pretty() << std::endl;
}
```

## Directory Structure

- `include/choochoo/` — Public headers
- `src/` — Library implementation
- `examples/` — Usage examples
- `tests/` — Automated tests

<!--## License-->
