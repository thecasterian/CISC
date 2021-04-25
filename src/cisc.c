#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("please specify input file\n");
        return 0;
    }

    FILE *fp = fopen(argv[1], "r");
    struct token_array token_array = lexer(fp);

    return 0;
}
