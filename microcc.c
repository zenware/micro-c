#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *user_input;

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Tokenizer
typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF, // End of File
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  int64_t val;
  char *str; // TODO: Use a string implementation that carries it's length.
};

Token *token;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error_at(token->str, "expected '%c'", op); // TODO: Better translation from ruiu compilerbook
  }
  token = token->next;
}

int64_t expect_number(void) {
  if (token->kind != TK_NUM) {
    error_at(token->str, "expected a number");
  }
  int64_t val = token->val;
  token = token->next;
  return val;
}

bool at_eof(void) {
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize(void) {
  char *p = user_input; // NOTE: Depends on a global.
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace((unsigned char)*p)) {
      p++;
      continue;
    }

    // Punctuator
    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit((unsigned char)*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtoll(p, &p, 10);
      continue;
    }
    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

static void emit_constant(uint64_t value) {
  printf("  movz x1, #0x%04llx\n", (unsigned long long)(value & 0xffff));

  for (int shift = 16; shift < 64; shift += 16) {
    uint64_t chunk = (value >> shift) & 0xffff;

    if (chunk != 0) {
      printf("  movk x1, #0x%04llx, lsl #%d\n", (unsigned long long)chunk, shift);
    }
  }
}

// Parser
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM,
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *lhs; // Left-hand side
  Node *rhs; // Right-hand side
  int64_t val; // If kind = ND_NUM
};

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int64_t val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr(void);
Node *mul(void);
Node *primary(void);

// expr = mul ("+" mul | "-" mul)*
Node *expr(void) {
  Node *node = mul();

  for (;;) {
    if (consume('+')) {
      node = new_node_binary(ND_ADD, node, mul());
    } else if (consume('-')) {
      node = new_node_binary(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

// mul = primary ("*" primary | "/" primary)*
Node *mul(void) {
  Node *node = primary();

  for (;;) {
    if (consume('*')) {
      node = new_node_binary(ND_MUL, node, primary());
    } else if (consume('/')) {
      node = new_node_binary(ND_DIV, node, primary());
    } else {
      return node;
    }
  }
}

// primary = "(" expr ")" | num
Node *primary(void) {
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  return new_node_num(expect_number());
}

// Code Generator
static void push_x0(void) {
  printf("  str x0, [sp, #-16]!\n");
}
static void pop_x0(void) {
  printf("  ldr x0, [sp], #16\n");
}
static void pop_x1(void) {
  printf("  ldr x1, [sp], #16\n");
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    emit_constant((uint64_t)node->val);
    printf("  mov x0, x1\n");
    push_x0();
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  pop_x1();
  pop_x0();

  switch (node->kind) {
  case ND_ADD:
    printf("  add x0, x0, x1\n");
    break;
  case ND_SUB:
    printf("  sub x0, x0, x1\n");
    break;
  case ND_MUL:
    printf("  mul x0, x0, x1\n");
    break;
  case ND_DIV:
    // NOTE: sdiv 'n/0' = 0, C semantics are 'n/0' = exception
    printf("  sdiv x0, x0, x1\n");
    break;
  case ND_NUM:
    error("unexpected numeric mode");
  }

  push_x0();
}

int main(int argc, char **argv) {
  if (argc != 2 ) {
    error("%s: invalid number of arguments", argv[0]);
  }

  // Tokenize and parse.
  user_input = argv[1];
  token = tokenize();
  Node *node = expr();
  
  // Print out the first half of assembly
  printf(".globl _main\n");
  printf("_main:\n");

  // Traverse the AST to emit assembly.
  gen(node);
  pop_x0(); // Results on the top of the stack.
  printf("  ret\n");
  return 0;
}
