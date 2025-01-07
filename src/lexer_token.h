#ifndef PEACHTOKENS_H
#define PEACHTOKENS_H

#include "lexer.h"

// NULL terminated list of valid keywords
static char *keywords[] = {"unsigned",
                           "signed",
                           "char",
                           "short",
                           "int",
                           "long",
                           "float",
                           "double",
                           "void",
                           "struct",
                           "union",
                           "static",
                           "__ignore_typecheck",
                           "return",
                           "include",
                           "sizeof",
                           "if",
                           "else",
                           "while",
                           "for",
                           "do",
                           "break",
                           "continue",
                           "switch",
                           "case",
                           "default",
                           "goto",
                           "typedef",
                           "const",
                           "extern",
                           "restrict",
                           NULL};

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

// Creates a new token of type TOKEN_TYPE_KEYWORD
// Lexer MUST be set to the first character of the keyword.
struct token *token_keyword_create(struct lexer *l);

// Creates a new token of type TOKEN_TYPE_IDENTIFIER
// Lexer MUST be set to the first character of the identifier.
// If the identifier is determined to be a keyword a TOKEN_TYPE_KEYWORD token
// is returned.
struct token *token_identifier_create(struct lexer *l);

// Creates a new token of type TOKEN_TYPE_NEWLINE
// Lexer MUST be set to the newline character.
struct token *token_newline_create(struct lexer *l);

// Creates a new token of type TOKEN_TYPE_COMMENT.
// Lexer MUST be set to the SECOND character of the comment string, '/' or '*'
// in the single line, '//', or the multiline, '/*' comment declaration,
// respectively.
struct token *token_comment_create(struct lexer *l);

// Creates a new token of type TOKEN_TYPE_NUMBER representing the character
// within a quote pair.
// Lexer MUST be set to the first quote character.
struct token *token_quote_create(struct lexer*l);

#endif  // PEACHTOKENS_H
