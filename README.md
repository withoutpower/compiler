this is a simple compiler based on pku-minic.

the directory mainly constructed as following:

- src
  - ast.h : define the classes of ast and their operation the produce Koopa IR of text form
  - sysy.l : define the lex rules for sysy which produce tokens according to our rules
  - sysy.y : define the grammars of sysy and construct the AST of memory form
  - visit.h : this file define a serial of function to visit the memory form of Koopa IR. These functions use DFS to produce riscv code for each instruction of Koopa IR.
  - main.cpp : this is the place combining all above functions. It includes following things:
    1. produce a ast with sysy.y and ast.h
    2. use lib koopa.h to read a text form of Koopa IR and construct a memory form of it.
    3. convert IR into riscv code with visit.h .
- Makefile : define rules to complie the whole project and more things
- main.c : file to test our compiler.
