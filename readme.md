# Micro C

This project is ultimately intended to be a C implementation like one of the
following:
- https://github.com/alexfru/SmallerC/
- https://github.com/rui314/chibicc/

I would also like to self-bootstrap as early as possible. In that sense, the
start of this project can be considered a search for the subset of C required
to create a compiler for itself.


## Resources
- I am following along with the [compiler book](https://www.sigbus.info/compilerbook) written by the author of chibicc
  - However I am developing from an M-Series Mac, and Mac is already explicitly not supported by the book due to not supporting static linking.
  - Despite that I'm writing aarch64-darwin assembly as my output until it breaks.
  - I'm also considering LLVM-IR or something like fasmg macro-assembly as my compilation target.
- https://bernsteinbear.com/blog/linear-scan/
