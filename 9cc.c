#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "number of arguments is invalid.\n");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    // p を base を基数として文字列に変換、不可能ならば &p に格納
    printf("    mov rax, %ld\n", strtol(p, &p, 10));

    // p が不正ポインタにならない限り
    while (*p)
    {
        if (*p == '+')
        {
            p++;
            printf("    add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-')
        {
            p++;
            printf("    sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "unexpected letter: '%c'\n", *p);
        return 1;
    }

    printf("    ret\n");
    return 0;
}