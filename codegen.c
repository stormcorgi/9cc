#include "9cc.h"

// pushes the given node's address to the stack
void gen_addr(Node *node) {
  printf("  //gen_addr%d\n", node->name);
  if (node->kind == ND_LVAR) {
    int offset = (node->name - 'a' + 1) * 8;
    printf("    lea rax, [rbp-%d]\n", offset);
    printf("    push rax\n");
    return;
  }

  error("not an lvalue.");
}

void load() {
  printf("  //load\n");
  printf("   pop rax\n");
  printf("   mov rax, [rax]\n");
  printf("   push rax\n");
}

void store() {
  printf("  //store\n");
  printf("   pop rdi\n");
  printf("   pop rax\n");
  printf("   mov [rax], rdi\n");
  printf("   push rdi\n");
}

// 構造木を受け取るとスタックマシンを模してアセンブリを出力
void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("    push %d\n", node->val);
      return;
    case ND_RETURN:
      printf("//ND_RETURN\n");
      gen(node->lhs);
      printf("    pop rax\n");
      printf("    jmp .Lreturn\n");
      return;
    case ND_LVAR:
      printf("//ND_LVAR\n");
      gen_addr(node);
      load();
      return;
    case ND_ASSIGN:
      printf("//ND_ASSIGN\n");
      gen_addr(node->lhs);
      gen(node->rhs);
      store();
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("    pop rdi\n");
  printf("    pop rax\n");

  switch (node->kind) {
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
      printf("//ND_DIV\n");
      printf("    cqo\n");
      printf("    idiv rdi\n");
      break;
    case ND_EQ:
      printf("//ND_EQ\n");
      printf("    cmp rax, rdi\n");
      printf("    sete al\n");
      printf("    movzb rax, al\n");
      break;
    case ND_NE:
      printf("//ND_NE\n");
      printf("    cmp rax, rdi\n");
      printf("    setne al\n");
      printf("    movzb rax, al\n");
      break;
    case ND_LT:
      printf("//ND_LT\n");
      printf("    cmp rax, rdi\n");
      printf("    setl al\n");
      printf("    movzb rax, al\n");
      break;
    case ND_LE:
      printf("//ND_LE\n");
      printf("    cmp rax, rdi\n");
      printf("    setle al\n");
      printf("    movzb rax, al\n");
      break;
  }

  printf("    push rax\n");
}

void codegen(Node *node) {
  //アセンブリの前半部分
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // Prologue
  printf("//Prologue\n");
  printf("    push rbp\n");
  printf("    mov rbp, rsp\n");
  printf("    sub rsp, 208\n");

  // 抽象構文木を降りながらコード生成
  for (Node *n = node; n; n = n->next) {
    gen(n);
  }

  // Epilogue
  printf("//Epilogue\n");
  printf(".Lreturn:\n");
  printf("    mov rsp, rbp\n");
  printf("    pop rbp\n");
  printf("    ret\n");
}