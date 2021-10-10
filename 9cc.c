#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char *user_input;

// kinds of nodes of abstract syntax tree
typedef enum 
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

// type of node of abstract syntax free
typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

// node generator
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
    // node->lhs, node->rhs remain to be NULL
}

// kinds of token
typedef enum
{
    TK_RESERVED, // 記号
    TK_NUM,      // 整数
    TK_EOF,      // 入力の終わり
} TokenKind;

typedef struct Token Token;
// Token type
struct Token
{
    TokenKind kind; // kinds of token
    Token *next; // next token
    int val;
    char *str;
};

// current token
Token *token;

// function for error message
// same arguments for printf
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt); // 可変兆個の変数を ap に格納

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt); // 可変兆個の変数を ap に格納

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// if the next token is expected token (op),
// read one token and return true, if not, return false;
bool consume(char op)
{
    if(token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

// if the next token is expected token (op),
// read one token, if not, throw error
void expect(char op)
{
    if(token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "this token is not '%c'", op);
    token = token->next;
}

// if the next token is numerical value, read it and return it
// if not, throw error;
int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "its not number.");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

// create next token and link against cur
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// tokenize string p, and return it;
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p)
    {
        // skip space
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
        {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            // strtol returns prefix long int (e.g. "123+" -> "123")
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("cannot tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

// create function for each object apears in EMNF
/*
expr = mul ("+" mul | "-" mul)*
mul  = primary ("*" primary | "/" primary)*
primary = num | "("expr")"
*/

Node *mul();
Node *expr();
Node *primary();

Node *expr()
{
    Node *node = mul();

    for (;;)
    {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = primary();
    for (;;)
    {
        if (consume('*'))
            node = new_node(ND_MUL, node, primary());
        else if (consume('/'))
            node = new_node(ND_DIV, node, primary());
        else
            return node;
    }
}

Node *primary()
{
    if (consume('('))
    {
        Node *node = expr();
        expect(')'); // if there is ) after expr(), consume )
        return node;
    }

    return new_node_num(expect_number());
}

void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n"); // take from top of the stack, return to rdi
    printf("    pop rax\n");

    switch(node->kind)
    {
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "number of arguments is invalid.\n");
        return 1;
    }

    token = tokenize(argv[1]);
    user_input = argv[1];
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // generate codes by searching abstract syntax tree
    gen(node);

    // result for whole tree remains to be on the top of the stack
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}