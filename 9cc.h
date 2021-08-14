#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
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
  Var *next; // 次の変数 or NULL
  char *name; // 変数名
  int offset; // RBPからのオフセット(Relational Base Posetion)
};

typedef enum {
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_EQ,      // ==
  ND_NE,      // !=
  ND_LT,      // <
  ND_LE,      // <=
  ND_ASSIGN,  // =
  ND_VAR,     // variable
  ND_RETURN,  // "return"
  ND_EXPR_STMT,// Expression statement
  ND_NUM,     // Integer
} NodeKind;


// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind;  // ノードの型
  Node *next;     // 次のノード
  Node *lhs;      // 左辺
  Node *rhs;      // 右辺
  int val;        // kindがND_NUMの時だけ使う
  Var *var;      // kindがND_VARの時使用
};

typedef struct {
  Node *node;
  Var *locals;
  int stack_size;
} Program;

Program *program();

//
// codegen.c
//

void codegen(Program *prog);
