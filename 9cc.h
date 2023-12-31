#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;

//// tokenize.c ////

typedef enum {
    TK_IDENT,
    TK_PUNCT,
    TK_KEYWORD,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct  Token Token;

struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *loc;
    int len;
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);


//// parse.c ////

typedef struct Obj Obj;
struct Obj {
    Obj *next;
    char *name;
    int offset;
};

typedef struct Function Function;
struct Function {
    Node *body;
    Obj *locals;
    int stack_size;
};



typedef enum {
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_NEG,       // unary -
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // =
    ND_RETURN,    // "return"
    ND_IF,
    ND_FOR,       //for or while
    ND_BLOCK,     // {...}
    ND_EXPR_STMT, // Expression statement
    ND_VAR,       // Variable
    ND_NUM,       // Integer
} NodeKind;


struct Node {
    NodeKind kind;
    Node *next;
    Node *tok;
    
    Node *lhs;
    Node *rhs;

    //IF
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // Block
    Node *body;

    Obj *var;
    int val;
};

Function *parse(Token *tok);


//// cidegen.c ////

void codegen(Function *prog);