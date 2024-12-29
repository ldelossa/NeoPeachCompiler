#include "lexer_token.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "../helpers/buffer.h"
#include "../helpers/vector.h"
#include "lexer.h"

bool is_keyword(char *str) {
    char **keyword = keywords;
    while (*keyword) {
        if (strcmp(str, *keyword) == 0) return true;
        keyword++;
    }
    return false;
}

struct token *token_number_create(struct lexer *l) {
    struct token *tok = calloc(1, sizeof(struct token));
    tok->pos = l->pos;

    struct buffer *buf = buffer_create();
    for (char c = lexer_peek_char(l); (c >= '0' && c <= '9');
         c = lexer_peek_char(l)) {
        buffer_write(buf, lexer_next_char(l));
    }

    tok->type = TOKEN_TYPE_NUMBER;
    tok->llnum = atoll(buffer_ptr(buf));
    buffer_free(buf);
    return tok;
}

struct token *token_string_create(struct lexer *l) {
    struct token *tok = calloc(1, sizeof(struct token));
    tok->type = TOKEN_TYPE_STRING;
    tok->pos = l->pos;

    struct buffer *buf = buffer_create();

    // pop-off initial delimiter
    char delim = lexer_next_char(l);

    char c = lexer_peek_char(l);
    while (c != delim && (signed char)c != EOF) {
        if (c == '\\') {
            // TODO: handle escapes
        }

        buffer_write(buf, lexer_next_char(l));
        c = lexer_peek_char(l);
    }

    // write null char
    buffer_write(buf, '\0');

    // pop off final delimiter, or else the lexer would think another string
    // exists
    lexer_next_char(l);

    char *str_buff = calloc(buf->len, sizeof(char));
    memcpy(str_buff, buffer_ptr(buf), buf->len);
    tok->sval = str_buff;

    buffer_free(buf);

    return tok;
}

// Determine if `op` is an operator which supports being joined with another
// operator.
bool operator_is_joinable(char op) {
    return op == '+' || op == '-' || op == '>' || op == '<' || op == '|' ||
           op == '&' || op == '*' || op == '/' || op == '=';
}

#define VALID_JOINED_OPS_LEN 12

// Vector containing valid joined operators.
char operator_joined[VALID_JOINED_OPS_LEN][2] = {
    {'*', '='}, {'/', '='}, {'+', '+'}, {'-', '-'}, {'>', '='}, {'<', '='},
    {'|', '|'}, {'&', '&'}, {'+', '='}, {'-', '='}, {'>', '>'}, {'<', '<'}};

// Ensures the two byte string `joined_op` is a valid joined operator from
// vector `operator_joined`
void operator_validate_joined_op(struct lexer *l, char *joined_op) {
    for (int i = 0; i < VALID_JOINED_OPS_LEN; i++) {
        if (memcmp(operator_joined[i], joined_op, 2) == 0) return;
    }
    lex_error(l, LEXICAL_ANALYSIS_INVALID_JOINED_OPERATOR);
}

// Checks if 'op' is the '<' operator and if so determines if its being used
// in an include statement, if it is a String token with the contained included
// file is returned.
struct token *token_operator_is_include(struct lexer *l, char op) {
    if (op != '<') return NULL;

    // we need to see if the lexer has an INCLUDE keyword token on its stack
    // if it does we need to return a string token with the file name
    struct token *tok = vector_back_or_null(l->token_vec);
    if (!tok) return NULL;

    if (tok->type == TOKEN_TYPE_KEYWORD && strcmp(tok->sval, "include") == 0) {
        return token_string_create(l);
    };

    return NULL;
}

struct token *token_operator_create(struct lexer *l) {
    struct token *tok = calloc(1, sizeof(struct token));
    struct token *include = NULL;
    struct buffer *buf = buffer_create();

    // peek first to see if we actually need to make a string token for '<'
    // operator usage in '#include <x.h>'
    char op = lexer_peek_char(l);
    if ((include = token_operator_is_include(l, op))) return include;

    // either operator was not '<' or '<' was not being used in an include so
    // continue on parsing the operator...
    op = lexer_next_char(l);
    buffer_write(buf, op);

    if (operator_is_joinable(op)) {
        if (operator_is_joinable(lexer_peek_char(l))) {
            op = lexer_next_char(l);
            buffer_write(buf, op);
            operator_validate_joined_op(l, buffer_ptr(buf));
        }
    }

    // Open parens means new expression, so instruct the lexer to create a new
    // expr.
    if (op == '(') lexer_new_expression(l);

    buffer_write(buf, '\0');

    char *token_str = calloc(buf->len, sizeof(char));
    strcpy(token_str, buffer_ptr(buf));

    tok->type = TOKEN_TYPE_OPERATOR;
    tok->sval = token_str;
    tok->pos = l->pos;

    buffer_free(buf);

    return tok;
}

struct token *token_symbol_create(struct lexer *l) {
    struct token *tok = calloc(1, sizeof(struct token));
    char c = lexer_next_char(l);
    if (c == ')') lexer_finish_expression(l);

    tok->type = TOKEN_TYPE_SYMBOL;
    tok->cval = c;
    tok->pos = l->pos;
    return tok;
}

struct token *token_identifier_create(struct lexer *l) {
    struct token *tok = calloc(1, sizeof(struct token));
    struct buffer *buf = buffer_create();

    unsigned char c = lexer_peek_char(l);
    while (isalnum(c) || c == '_') {
        c = lexer_next_char(l);
        buffer_write(buf, c);
        c = lexer_peek_char(l);
    }

    // add null byte
    buffer_write(buf, '\0');

    char *token_str = calloc(buf->len, sizeof(char));
    strcpy(token_str, buffer_ptr(buf));

    if (is_keyword(buffer_ptr(buf))) {
        tok->type = TOKEN_TYPE_KEYWORD;
    } else {
        tok->type = TOKEN_TYPE_IDENTIFIER;
    }
    tok->sval = token_str;
    tok->pos = l->pos;

    buffer_free(buf);
    return tok;
}
