#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#include "utils.h"

enum token_type {
    /* Keywords. */
    TOKEN_AUTO,                 /* auto */
    TOKEN_BREAK,                /* break */
    TOKEN_CASE,                 /* case */
    TOKEN_CHAR,                 /* char */
    TOKEN_CONST,                /* const */
    TOKEN_CONTINUE,             /* continue */
    TOKEN_DEFAULT,              /* default */
    TOKEN_DO,                   /* do */
    TOKEN_DOUBLE,               /* double */
    TOKEN_ELSE,                 /* else */
    TOKEN_ENUM,                 /* enum */
    TOKEN_EXTERN,               /* extern */
    TOKEN_FLOAT,                /* float */
    TOKEN_FOR,                  /* for */
    TOKEN_GOTO,                 /* goto */
    TOKEN_IF,                   /* if */
    TOKEN_INLINE,               /* inline */
    TOKEN_INT,                  /* int */
    TOKEN_LONG,                 /* long */
    TOKEN_REGISTER,             /* register */
    TOKEN_RESTRICT,             /* restrict */
    TOKEN_RETURN,               /* return */
    TOKEN_SHORT,                /* short */
    TOKEN_SIGNED,               /* signed */
    TOKEN_SIZEOF,               /* sizeof */
    TOKEN_STATIC,               /* static */
    TOKEN_STRUCT,               /* struct */
    TOKEN_SWITCH,               /* switch */
    TOKEN_TYPEDEF,              /* typedef */
    TOKEN_UNION,                /* union */
    TOKEN_UNSIGNED,             /* unsigned */
    TOKEN_VOID,                 /* void */
    TOKEN_VOLATILE,             /* volatile */
    TOKEN_WHILE,                /* while */
    TOKEN_ALIGNAS,              /* _Alignas */
    TOKEN_ALIGNOF,              /* _Alignof */
    TOKEN_ATOMIC,               /* _Atomic */
    TOKEN_BOOL,                 /* _Bool */
    TOKEN_COMPLEX,              /* _Complex */
    TOKEN_GENERIC,              /* _Generic */
    TOKEN_IMAGINARY,            /* _Imaginary */
    TOKEN_NORETURN,             /* _Noreturn */
    TOKEN_STATIC_ASSERT,        /* _Static_assert */
    TOKEN_THREAD_LOCAL,         /* _Thread_local */

    /* Identifers. */
    TOKEN_IDENTIFER,            /* identifer */

    /* Constants. enumeration-constant is combined with identifier. */
    TOKEN_INT_CONST,            /* integer-constant */
    TOKEN_FLOAT_CONST,          /* floating-constant */
    TOKEN_CHAR_CONST,           /* character-constant */

    /* String literal. */
    TOKEN_STRING_LITERAL,

    /* Punctuators. */
    TOKEN_BRACKET_OPEN,         /* [ or <: */
    TOKEN_BRACKET_CLOSE,        /* ] or :> */
    TOKEN_PAREN_OPEN,           /* ( */
    TOKEN_PAREN_CLOSE,          /* ) */
    TOKEN_BRACE_OPEN,           /* { or <% */
    TOKEN_BRACE_CLOSE,          /* } or %> */
    TOKEN_PERIOD,               /* . */
    TOKEN_ARROW,                /* -> */
    TOKEN_TWO_PLUS,             /* ++ */
    TOKEN_TWO_MINUS,            /* -- */
    TOKEN_AMPERSAND,            /* & */
    TOKEN_ASTERISK,             /* * */
    TOKEN_PLUS,                 /* + */
    TOKEN_MINUS,                /* - */
    TOKEN_TILDE,                /* ~ */
    TOKEN_EXCLAMATION,          /* ! */
    TOKEN_SLASH,                /* / */
    TOKEN_PERCENT,              /* % */
    TOKEN_LSHIFT,               /* << */
    TOKEN_RSHIFT,               /* >> */
    TOKEN_LESS_THAN,            /* < */
    TOKEN_GREATER_THAN,         /* > */
    TOKEN_LEQ,                  /* <= */
    TOKEN_GEQ,                  /* >= */
    TOKEN_EQUAL,                /* == */
    TOKEN_NOT_EQUAL,            /* != */
    TOKEN_CARROT,               /* ^ */
    TOKEN_VERT_BAR,             /* | */
    TOKEN_TWO_AMPERSAND,        /* && */
    TOKEN_TWO_VERT_BAR,         /* || */
    TOKEN_QUESTION,             /* ? */
    TOKEN_COLON,                /* : */
    TOKEN_SEMICOLON,            /* ; */
    TOKEN_THREE_PERIOD,         /* ... */
    TOKEN_ASSIGN,               /* = */
    TOKEN_MUL_ASSIGN,           /* *= */
    TOKEN_DIV_ASSIGN,           /* /= */
    TOKEN_MOD_ASSIGN,           /* %= */
    TOKEN_ADD_ASSIGN,           /* += */
    TOKEN_SUB_ASSIGN,           /* -= */
    TOKEN_LSHIFT_ASSIGN,        /* <<= */
    TOKEN_RSHIFT_ASSIGN,        /* >>= */
    TOKEN_AND_ASSIGN,           /* &= */
    TOKEN_XOR_ASSIGN,           /* ^= */
    TOKEN_OR_ASSIGN,            /* |= */
    TOKEN_COMMA,                /* , */
    TOKEN_SHARP,                /* # or %: */
    TOKEN_TWO_SHARP,            /* ## or %:%: */

    /* Indeterminate. */
    TOKEN_INDETERMINATE,
};

struct token {
    enum token_type type;
    struct string str;
};

struct token_array {
    size_t len;
    size_t capacity;
    struct token *tokens;
};

struct token_array lexer(FILE *file);

#endif