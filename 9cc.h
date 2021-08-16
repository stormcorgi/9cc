#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;

//
// tokenize.c
//

// token kind
typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

// token type
typedef struct Token Token;
struct Token {
  TokenKind kind;  //トークンの型
  Token *next;     //次の入力トークン
  int val;         // kindがTK_NUMのときの値
  char *str;       // トークン文字列
  int len;         // トークンの長さ
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
Token *peek(char *s);
Token *consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();

extern char *user_input;
extern Token *token;

//
// parse.c
//

// Local Variable type
typedef struct Var Var;
struct Var {
  char *name;  // 変数名
  Type *ty;    // 型
  int offset;  // RBPからのオフセット(Relational Base Posetion)
};

typedef struct VarList VarList;
struct VarList {
  VarList *next;
  Var *var;
};

typedef enum {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_ASSIGN,     // =
  ND_VAR,        // variable
  ND_RETURN,     // "return"
  ND_IF,         // if statement
  ND_EXPR_STMT,  // Expression statement
  ND_NUM,        // Integer
  ND_WHILE,      //"while"
  ND_FOR,        //"for"
  ND_BLOCK,      // {}
  ND_FUNCALL,    // Function call
  ND_ADDR,       // "&" address reference
  ND_DEREF,      // "*" for address dereference
  ND_NULL,       // empty statement
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind;  // ノードの型
  Node *next;     // 次のノード
  Type *ty;       // Type, e.g. int or pointer to int
  Token *tok;     // Representative token
  Node *lhs;      // 左辺
  Node *rhs;      // 右辺
  // if or while or for statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;
  // block
  Node *body;
  // function call
  char *funcname;
  Node *args;
  Var *var;  // kind == ND_VAR
  int val;   // kind == ND_NUM
};

typedef struct Function Function;
struct Function {
  Function *next;
  char *name;
  VarList *params;
  Node *node;
  VarList *locals;
  int stack_size;
};

Function *program();

//
// typing.c
//

typedef enum { TY_INT, TY_PTR } TypeKind;
struct Type {
  TypeKind kind;
  Type *base;
};

Type *int_type();
Type *pointer_to(Type *base);

void add_type(Function *prog);

//
// codegen.c
//

void codegen(Function *prog);
