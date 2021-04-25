#include "utils.h"
#include <assert.h>
#include <string.h>

void string_init(struct string *str) {
    str->len = 0;
    str->capacity = 0;
    str->arr = NULL;
}

void string_destroy(struct string *str) {
    str->len = -1;
    str->capacity = -1;
    free(str->arr);
    str->arr = NULL;
}

void string_append(struct string *str, char c) {
    assert(str->len != (size_t)-1);

    if (str->capacity == 0) {
        str->arr = malloc(16);
        str->capacity = 16;
    }
    else if (str->len+1 == str->capacity) {
        str->arr = realloc(str->arr, 2 * str->capacity);
        str->capacity *= 2;
    }
    str->arr[str->len] = c;
    str->len++;
    str->arr[str->len] = '\0';
}

void string_clear(struct string *str) {
    assert(str->len != (size_t)-1);

    str->len = 0;
    if (str->arr != NULL) str->arr[0] = '\0';
}

void string_copy(struct string *dest, struct string *src) {
    assert(!dest->len && !dest->capacity && !dest->arr);

    dest->len = src->len;
    dest->capacity = src->len + 1;
    dest->arr = malloc(src->len + 1);
    strncpy(dest->arr, src->arr, src->len+1);
}
