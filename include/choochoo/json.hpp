#pragma once

#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "value.hpp"


// Optionally, you can add convenience aliases or helper functions here
// For example:
// namespace choochoo::json {
//     using JsonLexer = Lexer;
//     using JsonParser = Parser;
//     using JsonValue = Value;
// }

//
// ChooChoo JSON Facade Header
//
// Usage:
//   #include <choochoo/json.hpp>
//
// This header includes all core components of the ChooChoo JSON library:
//   - Lexer: Tokenizes JSON input
//   - Parser: Parses tokens into a JSON value tree
//   - Value: Represents JSON values (object, array, string, number, etc.)
//
