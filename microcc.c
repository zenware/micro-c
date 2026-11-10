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

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit((unsigned char)*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtoll(p, &p, 10);
      continue;
    }
    error_at(p, "expected a number");
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

int main(int argc, char **argv) {
  if (argc != 2 ) {
    fprintf(stderr, "Not enough arguments\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize();
  
  printf(".globl _main\n");
  printf("_main:\n");
  emit_constant((uint64_t)expect_number());
  printf("  mov x0, x1\n");

  while (!at_eof()) {
    if (consume('+')) {
      emit_constant((uint64_t)expect_number());
      printf("  add x0, x0, x1\n");
      continue;
    }

    if (consume('-')) {
      emit_constant((uint64_t)expect_number());
      printf("  sub x0, x0, x1\n");
      continue;
    }

    error("unexpected token");
  }
  printf("  ret\n");
  return 0;
}
