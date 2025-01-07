#include "lexer.h"

#include <ctype.h>
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
        case LEXICAL_ANALYSIS_INVALID_EXPR_CLOSE:
            printf(
                "Invalid expression closure, did you close an expression that "
                "was not opened?\n");
            break;
        case LEXICAL_ANALYSIS_MULTILINE_COMMENT_NOT_CLOSED:
            printf("Multiline comment not closed\n");
            break;
        case LEXICAL_ANALYSIS_QUOTE_NOT_CLOSED:
            printf("Quote not closed\n");
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
    case '\t'

#define OPERATOR_OR_COMMENT_CASE \
    case '+':                    \
    case '-':                    \
    case '*':                    \
    case '>':                    \
    case '<':                    \
    case '^':                    \
    case '%':                    \
    case '!':                    \
    case '=':                    \
    case '~':                    \
    case '|':                    \
    case '&':                    \
    case '(':                    \
    case '[':                    \
    case ',':                    \
    case '.':                    \
    case '/':                    \
    case '?'

#define SYMBOL_CASE \
    case '{':       \
    case '}':       \
    case ':':       \
    case ';':       \
    case '#':       \
    case '\\':      \
    case ')':       \
    case ']'

#define STRING_CASE case '"'

#define NEWLINE_CASE case '\n'

#define COMMENT_CASE case '/'

#define QUOTE_CASE case '\''

#define EOF_CASE case EOF

#define DEFAULT_CASE default

struct lexer *lexer_create(struct compiler *c) {
    struct lexer *l = calloc(1, sizeof(struct lexer));
    l->compiler = c;
    return l;
};

bool valid_identifier_char(char c) { return (isalpha(c) || c == '_'); }

// peeks at the next char in the stream the lexer is parsing.
// the char will indicate what token to create and if it does not we exit
// our program with an unrecognized character error.
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
    OPERATOR_OR_COMMENT_CASE: {
        // handle '/' indicating a comment, not the division operator
        if (c == '/') {
            // discard the first '/' since we need to read the next char
            lexer_next_char(lexer);
            // peek at the next char, can be '/' and '*' if its a comment, any
            // other value indicates its a division operator
            char next_c = lexer_peek_char(lexer);
            if (next_c == '/' || next_c == '*') {
                tok = token_comment_create(lexer);
                break;
            }
            // its not a comment, but we moved the lexer one past the original
            // '/' operator, we push it back onto the stream, since
            // `token_operator_create` expects the lexer to ready to read the
            // current operator.
            lexer_push_char(lexer, '/');
        }
        tok = token_operator_create(lexer);
        break;
    }
    SYMBOL_CASE:
        tok = token_symbol_create(lexer);
        break;
    NEWLINE_CASE:
        tok = token_newline_create(lexer);
        break;
    WHITESPACE_CASE: {
        // last read token needs indication that white space was next
        struct token *last_token = vector_back_or_null(lexer->token_vec);
        last_token->whitespace = true;
        // discard this token
        lexer_next_char(lexer);
        // recurse here so this function still returns the next token,
        // if you return NULL here iteration would stop prematurely.
        return lexer_read_next_token(lexer);
    } break;
    QUOTE_CASE:
        tok = token_quote_create(lexer);
        break;
    EOF_CASE:
        // parsing done...
        return NULL;
    DEFAULT_CASE:
        if (valid_identifier_char(c))
            return tok = token_identifier_create(lexer);

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

void lexer_finish_expression(struct lexer *lexer) {
    lexer->current_expression_count--;
    if (lexer->current_expression_count < 0)
        lex_error(lexer, LEXICAL_ANALYSIS_INVALID_EXPR_CLOSE);
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
