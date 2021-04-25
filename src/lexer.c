#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define DEBUG 0

#define ADV(c) ((*(c))++)
#define APPADV(str, c) string_append(&(str), *((*(c))++))

static const char *keyword_table[] = {
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "register",
    "restrict",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
    "_Alignas",
    "_Alignof",
    "_Atomic",
    "_Bool",
    "_Complex",
    "_Generic",
    "_Imaginary",
    "_Noreturn",
    "_Static_assert",
    "_Thread_local",
};

static const char *punctuator_table[] = {
    "[",
    "]",
    "(",
    ")",
    "{",
    "}",
    ".",
    "->",
    "++",
    "--",
    "&",
    "*",
    "+",
    "-",
    "~",
    "!",
    "/",
    "%",
    "<<",
    ">>",
    "<",
    ">",
    "<=",
    ">=",
    "==",
    "!=",
    "^",
    "|",
    "&&",
    "||",
    "?",
    ":",
    ";",
    "...",
    "=",
    "*=",
    "/=",
    "%=",
    "+=",
    "-=",
    "<<=",
    ">>=",
    "&=",
    "^=",
    "|=",
    ",",
    "#",
    "##",
};

static const enum token_type NUM_KEYWORDS = 44;

static void token_array_init(struct token_array *tokarr);
static void token_array_append(struct token_array *tokarr, struct token tok);

static bool is_identifier_nondigit(int c);
static bool is_identifier(int c);
static bool is_nonzero_digit(int c);
static bool is_octal_digit(int c);
static bool is_simple_escape_sequence_character(int c);
static bool is_floating_suffix(int c);

static int read_keyword_or_identifier(char **c, struct token *tok);
static int read_integer_constant(char **c, struct token *tok);
static int read_floating_constant(char **c, struct token *tok);
static int read_character_constant(char **c, struct token *tok);
static int read_string_literal(char **c, struct token *tok);
static int read_punctuator(char **c, struct token *tok);

static int read_universal_character_name(char **c, struct string *str);
static int read_c_char_sequence(char **c, struct string *str);
static int read_s_char_sequence(char **c, struct string *str);

static void debug_print_token(struct token t);

struct token_array lexer(FILE *file) {
    /* Current line. */
    char *line = NULL;
    size_t line_len = 0;
    /* Current character. */
    char *c;
    /* Current token string. */
    struct string str;
    /* Current token. */
    struct token tok;
    /* Return. */
    struct token_array tokarr;

    string_init(&str);
    string_init(&tok.str);
    token_array_init(&tokarr);

    while (getline(&line, &line_len, file) != EOF) {
        c = line;

        while (1) {
            while (isspace(*c)) c++;
            if (*c == '\0') break;

            tok.type = TOKEN_INDETERMINATE;
            string_init(&tok.str);

            if (read_keyword_or_identifier(&c, &tok)
                && read_integer_constant(&c, &tok)
                && read_floating_constant(&c, &tok)
                && read_character_constant(&c, &tok)
                && read_string_literal(&c, &tok)
                && read_punctuator(&c, &tok)) {
                printf("error\n");
                return tokarr;
            }

            token_array_append(&tokarr, tok);
        }
    }

#if DEBUG
    for (size_t i = 0; i < tokarr.len; i++)
        debug_print_token(tokarr.tokens[i]);
    fprintf(stderr, "\n");
#endif

    return tokarr;
}

static void token_array_init(struct token_array *tokarr) {
    tokarr->len = 0;
    tokarr->capacity = 8;
    tokarr->tokens = malloc(sizeof(struct token) * 8);
}

static void token_array_append(struct token_array *tokarr, struct token tok) {
    if (tokarr->len == tokarr->capacity) {
        tokarr->tokens = realloc(tokarr->tokens, sizeof(struct token) * tokarr->capacity*2);
        tokarr->capacity *= 2;
    }
    tokarr->tokens[tokarr->len] = tok;
    tokarr->len++;
}

static bool is_identifier_nondigit(int c) {
    return isalpha(c) || c == '_';
}

static bool is_identifier(int c) {
    return isalnum(c) || c == '_';
}

static bool is_nonzero_digit(int c) {
    return isdigit(c) && c != '0';
}

static bool is_octal_digit(int c) {
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5'
           || c == '6' || c == '7';
}

static bool is_simple_escape_sequence_character(int c) {
    return c == '\'' || c == '\"' || c == '?' || c == '\\' || c == 'a'
           || c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't'
           || c == 'v';
}

static bool is_floating_suffix(int c) {
    return c == 'f' || c == 'l' || c == 'F' || c == 'L';
}

static int read_keyword_or_identifier(char **c, struct token *tok) {
    char *const co = *c;
    struct string str;

    string_init(&str);

    /* Must start with a nondigit identifier character. */
    if (!is_identifier_nondigit(**c))
        goto error;

    /* Read whole token. */
    while (is_identifier(**c)) APPADV(str, c);

    /* If there is a quote, it's an error. */
    if (**c == '\'' || **c == '\"') goto error;

    /* Is this identifier? */
    for (enum token_type i = 0; i < NUM_KEYWORDS; i++)
        if (!strcmp(str.arr, keyword_table[i])) {
            tok->type = i;
            break;
        }
    if (tok->type == TOKEN_INDETERMINATE)
        tok->type = TOKEN_IDENTIFER;

    /* Copy string. */
    string_copy(&tok->str, &str);

    string_destroy(&str);
    return 0;

error:
    string_destroy(&str);
    *c = co;
    return -1;
}

static int read_integer_constant(char **c, struct token *tok) {
    char *const co = *c;
    struct string str;

    string_init(&str);
    tok->type = TOKEN_INT_CONST;

    /* Must start with a digit. */
    if (!isdigit(**c)) goto error;

    /* Starting with a nonzero-digit: decimal-constant. */
    if (is_nonzero_digit(**c))
        while (isdigit(**c)) APPADV(str, c);

    /* Starting with a zero: octal-constant or hexadecimal-constant. */
    else if (**c == '0') {
        APPADV(str, c);

        /* Starting with 0x or 0X: hexadecimal-constant. */
        if (**c == 'x' || **c == 'X') {
            APPADV(str, c);
            while (isxdigit(**c)) APPADV(str, c);

            /* If there is no following hexadecimal-digit, it's an error. */
            if (*(*c-1) == 'x' || *(*c-1) == 'X')
                goto error;
        }

        /* Otherwise: octal-constant. */
        else
            while (is_octal_digit(**c)) APPADV(str, c);
    }

    /* Read integer-suffix */
    if (**c == 'u' || **c == 'U') {
        APPADV(str, c);
        if (**c == 'l' || **c == 'L') {
            APPADV(str, c);
            if (**c == *(*c-1)) APPADV(str, c);
        }
    } else if (**c == 'l' || **c == 'L') {
        APPADV(str, c);
        if (**c == *(*c-1)) APPADV(str, c);
        if (**c == 'u' || **c == 'U') APPADV(str, c);
    }

    /* If there is trailing identifier character, it's an error. */
    if (is_identifier(**c))
        goto error;

    /* If there is a period, it's an error; it's floating-constant. */
    if (**c == '.')
        goto error;

    /* Copy string. */
    string_copy(&tok->str, &str);

    string_destroy(&str);
    return 0;

error:
    string_destroy(&str);
    *c = co;
    return -1;
}

static int read_floating_constant(char **c, struct token *tok) {
    char *const co = *c;
    struct string str;
    int cnt;
    bool digit_seq_only;

    string_init(&str);
    tok->type = TOKEN_FLOAT_CONST;

    /* Starting with 0x or 0X: hexadecimal-floating-constant. */
    if (**c == '0' && (*(*c+1) == 'x' || *(*c+1) == 'X')) {
        APPADV(str, c);
        APPADV(str, c);

        /* hexadecimal-digit-sequence
           | hexadecimal-digit-sequence .
           | . hexadecimal-digit-sequence
           | hexadecimal-digit-sequence . hexadecimal-digit-sequence */
        cnt = 0;
        while (isxdigit(**c)) APPADV(str, c), cnt++;
        if (**c == '.') {
            APPADV(str, c), cnt++;
            while (isxdigit(**c)) APPADV(str, c), cnt++;
            if (cnt == 1) goto error;
        } else if (cnt == 0) goto error;

        /* binary-exponent-part. */
        if (**c != 'p' && **c != 'P') goto error;
        APPADV(str, c);
        if (**c == '+' || **c == '-') APPADV(str, c);
        if (!isdigit(**c)) goto error;
        while (isdigit(**c)) APPADV(str, c);
    }
    /* Otherwise: decimal floating-constant. */
    else {
        /* digit-sequence
           | digit-sequence .
           | . digit-sequence
           | digit-sequence . digit-sequence */
        cnt = 0;
        digit_seq_only = false;
        while (isdigit(**c)) APPADV(str, c), cnt++;
        if (**c == '.') {
            APPADV(str, c), cnt++;
            while (isdigit(**c)) APPADV(str, c), cnt++;
            if (cnt == 1) goto error;
        } else {
            if (cnt == 0) goto error;
            digit_seq_only = true;
        }

        /* exponent-part. */
        if (digit_seq_only && **c != 'e' && **c != 'E') goto error;
        if (**c == 'e' || **c == 'E') {
            APPADV(str, c);
            if (**c == '+' || **c == '-') APPADV(str, c);
            if (!isdigit(**c)) goto error;
            while (isdigit(**c)) APPADV(str, c);
        }
    }

    /* Read floating-suffix. */
    if (is_floating_suffix(**c)) APPADV(str, c);

    /* If there is trailing identifier character, it's an error. */
    if (is_identifier(**c))
        goto error;

    /* If there is a period, it's an error. */
    if (**c == '.')
        goto error;

    /* Copy string. */
    string_copy(&tok->str, &str);

    string_destroy(&str);
    return 0;

error:
    string_destroy(&str);
    *c = co;
    return -1;
}

static int read_character_constant(char **c, struct token *tok) {
    char *const co = *c;
    struct string str;

    string_init(&str);
    tok->type = TOKEN_CHAR_CONST;

    /* Must start with ', L', u', or U'. */
    if (**c == '\'') APPADV(str, c);
    else if ((**c == 'L' || **c == 'u' || **c == 'U') && *(*c+1) == '\'') {
        APPADV(str, c);
        APPADV(str, c);
    }
    else goto error;

    /* Read c-char-sequence. */
    if (read_c_char_sequence(c, &str)) goto error;

    /* Must end with a single quote. */
    if (**c != '\'') goto error;
    APPADV(str, c);

    /* Copy string. */
    string_copy(&tok->str, &str);

    string_destroy(&str);
    return 0;

error:
    string_destroy(&str);
    *c = co;
    return -1;
}

static int read_string_literal(char **c, struct token *tok) {
    char *const co = *c;
    struct string str;

    string_init(&str);
    tok->type = TOKEN_STRING_LITERAL;

    /* Must start with ", u8", u", U", or L". */
    if (**c == '\"') APPADV(str, c);
    else if ((**c == 'u' || **c == 'U' || **c == 'L') && *(*c+1) == '\"') {
        APPADV(str, c);
        APPADV(str, c);
    }
    else if (**c == 'u' && *(*c+1) == '8' && *(*c+2) == '\"') {
        APPADV(str, c);
        APPADV(str, c);
        APPADV(str, c);
    }
    else goto error;

    /* Read s-char-sequence. */
    if (read_s_char_sequence(c, &str)) goto error;

    /* Must end with a double quote. */
    if (**c != '\"') goto error;
    APPADV(str, c);

    /* Copy string. */
    string_copy(&tok->str, &str);

    string_destroy(&str);
    return 0;

error:
    string_destroy(&str);
    *c = co;
    return -1;
}

static int read_punctuator(char **c, struct token *tok) {
    char *const co = *c;

    if (**c == '[') {
        ADV(c);
        tok->type = TOKEN_BRACKET_OPEN;
    } else if (**c == ']') {
        ADV(c);
        tok->type = TOKEN_BRACKET_CLOSE;
    } else if (**c == '(') {
        ADV(c);
        tok->type = TOKEN_PAREN_OPEN;
    } else if (**c == ')') {
        ADV(c);
        tok->type = TOKEN_PAREN_CLOSE;
    } else if (**c == '{') {
        ADV(c);
        tok->type = TOKEN_BRACE_OPEN;
    } else if (**c == '}') {
        ADV(c);
        tok->type = TOKEN_BRACE_CLOSE;
    } else if (**c == '.') {
        ADV(c);
        if (**c == '.' && *(*c+1) == '.') {
            ADV(c); ADV(c);
            tok->type = TOKEN_THREE_PERIOD;
        } else
            tok->type = TOKEN_PERIOD;
    } else if (**c == '-') {
        ADV(c);
        if (**c == '>') {
            ADV(c);
            tok->type = TOKEN_ARROW;
        } else if (**c == '-') {
            ADV(c);
            tok->type = TOKEN_TWO_MINUS;
        } else if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_SUB_ASSIGN;
        } else
            tok->type = TOKEN_MINUS;
    } else if (**c == '+') {
        ADV(c);
        if (**c == '+') {
            ADV(c);
            tok->type = TOKEN_TWO_PLUS;
        } else if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_ADD_ASSIGN;
        } else
            tok->type = TOKEN_PLUS;
    } else if (**c == '&') {
        ADV(c);
        if (**c == '&') {
            ADV(c);
            tok->type = TOKEN_TWO_AMPERSAND;
        } else if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_AND_ASSIGN;
        } else
            tok->type = TOKEN_AMPERSAND;
    } else if (**c == '*') {
        ADV(c);
        if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_MUL_ASSIGN;
        } else
            tok->type = TOKEN_ASTERISK;
    } else if (**c == '~') {
        ADV(c);
        tok->type = TOKEN_TILDE;
    } else if (**c == '!') {
        ADV(c);
        if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_NOT_EQUAL;
        } else
            tok->type = TOKEN_EXCLAMATION;
    } else if (**c == '/') {
        ADV(c);
        if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_DIV_ASSIGN;
        } else
            tok->type = TOKEN_SLASH;
    } else if (**c == '%') {
        ADV(c);
        if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_MOD_ASSIGN;
        } else if (**c == '>') {
            ADV(c);
            tok->type = TOKEN_BRACE_CLOSE;
        } else if (**c == ':' && *(*c+1) != '%') {
            ADV(c);
            tok->type = TOKEN_SHARP;
        } else if (**c == ':' && *(*c+1) == '%' && *(*c+2) == ':') {
            ADV(c); ADV(c); ADV(c);
            tok->type = TOKEN_TWO_SHARP;
        } else
            tok->type = TOKEN_PERCENT;
    } else if (**c == '<') {
        ADV(c);
        if (**c == '<' && *(*c+1) != '=') {
            ADV(c);
            tok->type = TOKEN_LSHIFT;
        } else if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_LEQ;
        } else if (**c == '<' && *(*c+1) == '=') {
            ADV(c); ADV(c);
            tok->type = TOKEN_LSHIFT_ASSIGN;
        } else if (**c == ':') {
            ADV(c);
            tok->type = TOKEN_BRACKET_OPEN;
        } else if (**c == '%') {
            ADV(c);
            tok->type = TOKEN_BRACE_OPEN;
        } else
            tok->type = TOKEN_LESS_THAN;
    } else if (**c == '>') {
        ADV(c);
        if (**c == '>' && *(*c+1) != '=') {
            ADV(c);
            tok->type = TOKEN_RSHIFT;
        } else if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_GEQ;
        } else if (**c == '>' && *(*c+1) == '=') {
            ADV(c); ADV(c);
            tok->type = TOKEN_RSHIFT_ASSIGN;
        } else
            tok->type = TOKEN_GREATER_THAN;
    } else if (**c == '=') {
        ADV(c);
        if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_EQUAL;
        } else
            tok->type = TOKEN_ASSIGN;
    } else if (**c == '^') {
        ADV(c);
        if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_XOR_ASSIGN;
        } else
            tok->type = TOKEN_CARROT;
    } else if (**c == '|') {
        ADV(c);
        if (**c == '|') {
            ADV(c);
            tok->type = TOKEN_TWO_VERT_BAR;
        } else if (**c == '=') {
            ADV(c);
            tok->type = TOKEN_OR_ASSIGN;
        } else
            tok->type = TOKEN_VERT_BAR;
    } else if (**c == '?') {
        ADV(c);
        tok->type = TOKEN_QUESTION;
    } else if (**c == ':') {
        ADV(c);
        if (**c == '>') {
            ADV(c);
            tok->type = TOKEN_BRACKET_CLOSE;
        } else
            tok->type = TOKEN_COLON;
    } else if (**c == ';') {
        ADV(c);
        tok->type = TOKEN_SEMICOLON;
    } else if (**c == ',') {
        ADV(c);
        tok->type = TOKEN_COMMA;
    } else if (**c == '#') {
        ADV(c);
        if (**c == '#') {
            ADV(c);
            tok->type = TOKEN_TWO_SHARP;
        } else
            tok->type = TOKEN_SHARP;
    } else goto error;

    return 0;

error:
    *c = co;
    return -1;
}

static int read_universal_character_name(char **c, struct string *str) {
    printf("not yet implemented\n");
    return -1;
}

static int read_c_char_sequence(char **c, struct string *str) {
    const int leno = str->len;

    while (**c != '\'') {
        /* c-char-sequence cannot contain a new-line character. */
        if (**c == '\n') goto error;

        /* escape-sequence. */
        if (**c == '\\') {
            APPADV(*str, c);

            /* simple-escape-sequence. */
            if (is_simple_escape_sequence_character(**c))
                APPADV(*str, c);
            /* octal-escape-squence. */
            else if (is_octal_digit(**c)) {
                APPADV(*str, c);
                if (is_octal_digit(**c)) {
                    APPADV(*str, c);
                    if (is_octal_digit(**c)) APPADV(*str, c);
                }
            }
            /* hexadecimal-escape-sequencel. */
            else if (**c == 'x') {
                APPADV(*str, c);
                while (isxdigit(**c)) APPADV(*str, c);
            }
            /* universal-character-name. */
            else if (!read_universal_character_name(c, str)) {}
            /* Error. */
            else goto error;
        }

        /* Any other character is OK. */
        else APPADV(*str, c);
    }

    return 0;

error:
    str->len = leno;
    str->arr[leno] = '\0';
    return -1;
}

static int read_s_char_sequence(char **c, struct string *str) {
    const int leno = str->len;

    while (**c != '\"') {
        /* c-char-sequence cannot contain a new-line character. */
        if (**c == '\n') goto error;

        /* escape-sequence. */
        if (**c == '\\') {
            APPADV(*str, c);

            /* simple-escape-sequence. */
            if (is_simple_escape_sequence_character(**c))
                APPADV(*str, c);
            /* octal-escape-squence. */
            else if (is_octal_digit(**c)) {
                APPADV(*str, c);
                if (is_octal_digit(**c)) {
                    APPADV(*str, c);
                    if (is_octal_digit(**c)) APPADV(*str, c);
                }
            }
            /* hexadecimal-escape-sequencel. */
            else if (**c == 'x') {
                APPADV(*str, c);
                while (isxdigit(**c)) APPADV(*str, c);
            }
            /* universal-character-name. */
            else if (!read_universal_character_name(c, str)) {}
            /* Error. */
            else goto error;
        }

        /* Any other character is OK. */
        else APPADV(*str, c);
    }

    return 0;

error:
    str->len = leno;
    str->arr[leno] = '\0';
    return -1;
}

static void debug_print_token(struct token t) {
    if (t.type < NUM_KEYWORDS) {
        fprintf(stderr, "keyword:%s ", keyword_table[t.type]);
        return;
    }
    if (t.type >= TOKEN_BRACKET_OPEN && t.type <= TOKEN_TWO_SHARP) {
        fprintf(stderr, "%s ", punctuator_table[t.type - TOKEN_BRACKET_OPEN]);
        return;
    }
    switch (t.type) {
    case TOKEN_IDENTIFER:
        fprintf(stderr, "identifier:%s ", t.str.arr);
        break;
    case TOKEN_INT_CONST:
        fprintf(stderr, "integer-constant:%s ", t.str.arr);
        break;
    case TOKEN_FLOAT_CONST:
        fprintf(stderr, "floating-constant:%s ", t.str.arr);
        break;
    case TOKEN_CHAR_CONST:
        fprintf(stderr, "character-constant:%s ", t.str.arr);
        break;
    case TOKEN_STRING_LITERAL:
        fprintf(stderr, "string-literal:%s ", t.str.arr);
        break;
    default:;
    }
}
