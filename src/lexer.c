#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>

#include "../helpers/buffer.h"
#include "../helpers/vector.h"
#include "lexer_token.h"

void lex_error(struct lexer *lex, enum lex_errors e) {
    printf("[ERROR]: ");
    switch (e) {
        case LEXICAL_ANALYSIS_INPUT_ERROR:
            printf("Lexical analysis input error\n");
            break;
        case LEXICAL_ANALYSIS_INVALID_JOINED_OPERATOR:
            printf("Invalid joined operator\n");
            break;
        default:
            printf("Unknown error\n");
            break;
    }

    printf("Lexical error at line: %d, col: %d at file: %s\n", lex->pos.line,
           lex->pos.col, lex->pos.filename);
    exit(-1);
}

// LEXER Characters //
#define NUMERIC_CASE \
    case '0':        \
    case '1':        \
    case '2':        \
    case '3':        \
    case '4':        \
    case '5':        \
    case '6':        \
    case '7':        \
    case '8':        \
    case '9'

#define WHITESPACE_CASE \
    case ' ':           \
    case '\n'

#define OPERATOR_CASE_EXCLUDING_DIVISION \
    case '+':                            \
    case '-':                            \
    case '*':                            \
    case '>':                            \
    case '<':                            \
    case '^':                            \
    case '%':                            \
    case '!':                            \
    case '=':                            \
    case '~':                            \
    case '|':                            \
    case '&':                            \
    case '(':                            \
    case '[':                            \
    case ',':                            \
    case '.':                            \
    case '?'

#define STRING_CASE case '"'

#define EOF_CASE case EOF

#define DEFAULT_CASE default

struct lexer *lexer_create(struct compiler *c) {
    struct lexer *l = calloc(1, sizeof(struct lexer));
    l->compiler = c;
    return l;
};

struct token *lexer_read_next_token(struct lexer *lexer) {
    struct token *tok = NULL;
    signed char c = lexer_peek_char(lexer);
    switch (c) {
    NUMERIC_CASE:
        tok = token_number_create(lexer);
        break;
    STRING_CASE:
        tok = token_string_create(lexer);
        break;
    OPERATOR_CASE_EXCLUDING_DIVISION:
        tok = token_operator_create(lexer);
        break;
    WHITESPACE_CASE:
        // handle white case scenario
        {
            // last read token needs indication that white space was next
            struct token *last_token = vector_back_or_null(lexer->token_vec);
            last_token->whitespace = true;
            // discard this token
            lexer_next_char(lexer);
            // recurse here so this function still returns the next token,
            // if you return NULL here iteration would stop prematurely.
            return lexer_read_next_token(lexer);
        }
        break;
    EOF_CASE:
        // parsing done...
        return NULL;
    DEFAULT_CASE:
        // we peeked at the char to find an unhandled token, so we need to
        // increment the lexers col to get the accurate col unknown token
        // is at
        lexer->pos.col += 1;
        lex_error(lexer, LEXICAL_ANALYSIS_INPUT_ERROR);
    }
    return tok;
}

int lexer_lex(struct lexer *lexer) {
    lexer->current_expression_count = 0;
    lexer->parantheses_buffer = NULL;
    lexer->pos.filename = lexer->compiler->cfile.abs_path;
    lexer->token_vec = vector_create(sizeof(struct token));
    struct token *token = lexer_read_next_token(lexer);
    while (token) {
        vector_push(lexer->token_vec, token);
        token = lexer_read_next_token(lexer);
    }
    return LEXICAL_ANALYSIS_ALL_OK;
};

void lexer_new_expression(struct lexer *lexer) {
    lexer->current_expression_count++;
    if (lexer->current_expression_count == 1)
        lexer->parantheses_buffer = buffer_create();
}

bool lexer_in_expression(struct lexer *lexer) {
    return lexer->current_expression_count > 0;
}

char lexer_next_char(struct lexer *lexer) {
    char c = getc(lexer->compiler->cfile.fp);
    lexer->pos.col += 1;
    if (c == '\n') {
        lexer->pos.line += 1;
        lexer->pos.col = 1;
    }
    return c;
};
char lexer_peek_char(struct lexer *lexer) {
    char c = getc(lexer->compiler->cfile.fp);
    lexer_push_char(lexer, c);
    return c;
};
void lexer_push_char(struct lexer *lexer, char c) {
    ungetc(c, lexer->compiler->cfile.fp);
};