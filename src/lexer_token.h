#ifndef PEACHTOKENS_H
#define PEACHTOKENS_H

#include "lexer.h"

// Creates a new token of type TOKEN_TYPE_NUMBER
// Lexer MUST be set to the first character of the numeric token.
struct token *token_number_create(struct lexer *l);

// Creates a new token of type TOKEN_TYPE_STRING
// Lexer MUST be set to the first character delimiter of the string and will
// read until EOF or the delimeter is found.
struct token *token_string_create(struct lexer *l);

// Creates a new token of type TOKEN_TYPE_IDENTIFIER
// Lexer MUST be set to the first character of the operator.
// If the '<' operator is encountered a check to see if it the `include` keyword
// preceeds it and if it does a TOKEN_TYPE_STRING token representing the
// included file is returned.
struct token *token_operator_create(struct lexer *l);

// Creates a new token of type TOKEN_TYPE_SYMBOL
// Lexer MUST be set to the first character of the symbol.
struct token *token_symbol_create(struct lexer *l);

#endif  // PEACHTOKENS_H
