#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

struct string {
    size_t len;
    size_t capacity;
    char *arr;
};

void string_init(struct string *str);
void string_destroy(struct string *str);

void string_append(struct string *str, char c);
void string_clear(struct string *str);
void string_copy(struct string *dest, struct string *src);

#endif