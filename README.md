# ChooChoo JSON

A modern, performant C++ JSON parser library that just keeps chuggin (like high speed rail).

## Installation

You can build and install the library using CMake:

```sh
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja -S . -B ./cmake-build-release
ninja -C ./cmake-build-release
ninja -C ./cmake-build-release install
```

This will install the library and headers to your system (see CMake install rules in `CMakeLists.txt`).

## API Overview

- **Lexer:** Tokenizes JSON input (supports both string and stream input).
- **Parser:** Parses tokens into a JSON value tree.
- **Value:** Represents JSON values (object, array, string, number, etc.).
- **Error Handling:** Uses `std::expected` for modern, explicit error reporting.
- **Iterator Support:** Iterate over arrays and objects using STL-style iterators and range-based for loops.
- **Streaming Support:** Parse JSON directly from any `std::istream` (e.g., file, network, stringstream).

## Examples

See the `examples/` directory for usage samples:

- [Basic Usage](examples/basic_usage.cpp): Parse and pretty-print JSON.
- [Error Handling](examples/error_handling.cpp): Handle and report parse errors.
- [Nested Access](examples/nested_access.cpp): Extract values from nested objects/arrays.
- [Iteration](examples/iteration.cpp): Iterate over arrays and objects.
- [Iterator Usage](examples/iterator_usage.cpp): STL-style iteration and algorithms.
- [Streaming Usage](examples/streaming_usage.cpp): Parse JSON directly from streams.



## Features

- Fast, standards-compliant JSON parsing
- Modern C++23 error handling with `std::expected`
- Easy to use API
- STL-style iterator support for arrays and objects
- Parse JSON from strings or any `std::istream` (streaming)
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

### Iterator Example

```cpp
// Iterate over array values
for (const auto& item : result.value().as_array()->get()) {
    // Use item.as_number(), item.as_string(), etc.
}

// Iterate over object key-value pairs
for (const auto& [key, value] : result.value().as_object()->get()) {
    std::cout << key << ": " << value.pretty() << std::endl;
}
```

### Streaming Example

```cpp
#include <sstream>
std::istringstream stream(R"({"streamed": true, "numbers": [1,2,3]})");
choochoo::json::Lexer lexer(stream);
choochoo::json::Parser parser(lexer);
auto result = parser.parse();
```

## Directory Structure

- `include/choochoo/` — Public headers
- `src/` — Library implementation
- `examples/` — Usage examples
- `tests/` — Automated tests

<!--## License-->
