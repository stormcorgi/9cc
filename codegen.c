#include "9cc.h"

// for if statements
int labelseq = 0;
// for func call with arguments
char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
// for func definition
char *funcname;

void gen(Node *node);

// pushes the given node's address to the stack
void gen_addr(Node *node) {
  switch (node->kind) {
    case ND_VAR:
      printf("    lea rax, [rbp-%d]\n", node->var->offset);
      printf("    push rax\n");
      return;
    case ND_DEREF:
      gen(node->lhs);
      return;
  }

  error_tok(node->tok, "not an lvalue.");
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
    case ND_NULL:
      return;
    case ND_NUM:
      printf("    push %d\n", node->val);
      return;
    case ND_VAR:
      printf("//ND_VAR\n");
      gen_addr(node);
      load();
      return;
    case ND_EXPR_STMT:
      gen(node->lhs);
      printf("    add rsp, 8\n");
      return;
    case ND_ASSIGN:
      printf("//ND_ASSIGN\n");
      gen_addr(node->lhs);
      gen(node->rhs);
      store();
      return;
    case ND_IF: {
      int seq = labelseq++;
      if (node->els) {
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je  .Lelse%d\n", seq);
        gen(node->then);
        printf("    jmp .Lend%d\n", seq);
        printf(".Lelse%d:\n", seq);
        gen(node->els);
        printf(".Lend%d:\n", seq);
      } else {
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je  .Lend%d\n", seq);
        gen(node->then);
        printf(".Lend%d:\n", seq);
      }
      return;
    }
    case ND_WHILE: {
      int seq = labelseq++;
      printf(".Lbegin%d:\n", seq);
      gen(node->cond);
      printf("    pop rax\n");
      printf("    cmp rax, 0\n");
      printf("    je  .Lend%d\n", seq);
      gen(node->then);
      printf("    jmp .Lbegin%d\n", seq);
      printf(".Lend%d:\n", seq);
      return;
    }
    case ND_FOR: {
      int seq = labelseq++;
      if (node->init) gen(node->init);
      printf(".Lbegin%d:\n", seq);
      if (node->cond) {
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je  .Lend%d\n", seq);
      }
      gen(node->then);
      if (node->inc) gen(node->inc);
      printf("    jmp .Lbegin%d\n", seq);
      printf(".Lend%d:\n", seq);
      return;
    }
    case ND_BLOCK:
      for (Node *n = node->body; n; n = n->next) gen(n);
      return;
    case ND_FUNCALL: {
      int nargs = 0;

      for (Node *arg = node->args; arg; arg = arg->next) {
        gen(arg);
        nargs++;
      }

      for (int i = nargs - 1; i >= 0; i--) {
        printf("    pop %s\n", argreg[i]);
      }

      // RSP must aligned on a 16byte boundary (ABI requirements).
      // pop moves RSP by 8byte, so RSP could aligned 16byte boundary in 50%
      int seq = labelseq++;
      printf("    mov rax, rsp\n");
      printf("    and rax, 15\n");
      printf("    jnz .Lcall%d\n", seq);
      // RSP aligned on 16byte boundary.
      printf("    mov rax, 0\n");
      printf("    call  %s\n", node->funcname);
      printf("    jmp .Lend%d\n", seq);
      // RSP is not aligned on 16byte, -8 rsp -> call func -> +8 rsp
      printf(".Lcall%d:\n", seq);
      printf("    sub rsp, 8\n");
      printf("    mov rax, 0\n");
      printf("    call %s\n", node->funcname);
      printf("    add rsp, 8\n");
      printf(".Lend%d:\n", seq);
      printf("    push rax\n");
      return;
    }
    case ND_ADDR:
      gen_addr(node->lhs);
      return;
    case ND_DEREF:
      gen(node->lhs);
      load();
      return;
    case ND_RETURN:
      printf("//ND_RETURN\n");
      gen(node->lhs);
      printf("    pop rax\n");
      printf("    jmp .Lreturn.%s\n", funcname);
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("    pop rdi\n");
  printf("    pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      if (node->ty->kind == TY_PTR) printf("    imul rdi, 8\n");
      printf("    add rax, rdi\n");
      break;
    case ND_SUB:
      if (node->ty->kind == TY_PTR) printf("    imul rdi, 8\n");
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

void codegen(Function *prog) {
  printf(".intel_syntax noprefix\n");

  for (Function *fn = prog; fn; fn = fn->next) {
    printf(".globl %s\n", fn->name);
    printf("%s:\n", fn->name);
    funcname = fn->name;

    // Prologue
    printf("//Prologue\n");
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", fn->stack_size);

    // 引数をスタックにプッシュ
    int i = 0;
    for (VarList *vl = fn->params; vl; vl = vl->next) {
      Var *var = vl->var;
      printf("    mov [rbp-%d], %s\n", var->offset, argreg[i++]);
    }

    // 抽象構文木を降りながらコード生成
    for (Node *node = fn->node; node; node = node->next) {
      gen(node);
    }

    // Epilogue
    printf("//Epilogue\n");
    printf(".Lreturn.%s:\n", funcname);
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
  }
}