#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2 ) {
    fprintf(stderr, "Not enough arguments\n");
    return 1;
  }
  printf(".globl _main\n");
  printf("_main:\n");
  printf("  mov x0, #%d\n", atoi(argv[1]));
  printf("  ret\n");
  return 0;
}
