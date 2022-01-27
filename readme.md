# Micro C

This project is ultimately intended to be a C implementation like one of the
following:
- https://github.com/alexfru/SmallerC/
- https://github.com/rui314/chibicc/

I would also like to self-bootstrap as early as possible. In that sense, the
start of this project can be considered a search for the subset of C required
to create a compiler for itself.

I am following along with the [compiler book](https://www.sigbus.info/compilerbook) written by the author of chibicc, however I am developing from an M1 Mac, and Mac is already explicitly not supported by the book due to not supporting static linking. I will try to get around that by compiling to LLVM-IR as my compilation target.