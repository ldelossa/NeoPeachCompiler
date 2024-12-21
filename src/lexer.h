#ifndef PEACHLEXER_H
#define PEACHLEXER_H

#include <stdbool.h>

#include "compiler.h"

enum lex_errors {
    LEXICAL_ANALYSIS_ALL_OK,
    LEXICAL_ANALYSIS_INVALID_JOINED_OPERATOR,
    LEXICAL_ANALYSIS_INPUT_ERROR
};

enum {
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_NEWLINE,
};

struct token {
    int type;
    int flags;
    struct pos pos;

    union {
        char cval;
        const char *sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        void *any;
    };

    // True if their is whitespace between next token.
    bool whitespace;

    // Points to string between two brackets.
    // e.g: (10+20+30)
    //       ^ char *between_brackets points to first char within the brackets
    // useful for debugging
    const char *between_brackets;
};

struct lexer {
    struct pos pos;
    struct vector *token_vec;
    struct compiler *compiler;

    int current_expression_count;
    struct buffer *parantheses_buffer;

    void *private;
};

struct lexer *lexer_create(struct compiler *c);

// Start lexing the file configured for the lexer's embedded compiler instance.
// Returns LEXICAL_ANALYSIS_ALL_OK if no errors were encountered.
// Any errors will OS exit our compiler with details on stderr.
int lexer_lex(struct lexer *lexer);

// Start a new expression.
void lexer_new_expression(struct lexer *lexer);
// Informs the caller if the lexer is currently in an expression.
bool lexer_in_expression(struct lexer *lexer);

// Returns the next character of the file stream the lexer is currently parsing,
// moving the lexer to the next character in the stream.
char lexer_next_char(struct lexer *lexer);
// Returns the next character of the file stream the lexer is currently parsing,
// without moving the lexer to the next character in the stream.
char lexer_peek_char(struct lexer *lexer);
// Pushes a character back onto the lexer's stream.
// The character is only pushed to the in-memory stream and does not result int
// any write to the file being lexed.
void lexer_push_char(struct lexer *lexer, char c);

// Write an error to stderr for the given lexer error enum and exit the process.
void lex_error(struct lexer *lex, enum lex_errors e);

#endif  // PEACHLEXER_H
