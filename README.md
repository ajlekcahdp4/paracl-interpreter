# 01-paracc

Interpreter implementation for ParaCL, a custom C-like language.

## 0. What is ParaCL

ParaCL is a C-like language for teaching about compiler frontend development.

### Example program in ParaCL

```sh
n = ?;
fact = 1;

while (n > 0)
{
  fact = fact * n;
  n = n - 1;
}

print fact;
```

### More complex example.

In ParaCL assignments are chainable and multiple variables can be declared in a single line. Futhermore, like in C, assignment is a statement as well as an expression. Here is a more complex examples which prints the absolute value of the input number in a loop:

```sh
// Testing assignments as expressions
while ((x = ?) != 0) {
  if ((y = ?) > 0) {
    print y;
  } else {
    print -y;
  }
}
```

As a bonus, single line C++ style comments are allowed.

## 1. How to build

This interpreter relies on GNU Bison and Flex. You have to have them installed to build this project.

### Linux
```sh
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release
cd build/
make -j12
```

## 2. ParaCL Compiler (pclc)
The _pclc_ binary is the brain of the whole interpreter. It compiles the source file into an executable for the ParaCL VM and executes it. It's possible to skip the execution and dump the disassembled binary or write it to file.
There is a standalone VM executable _pclvm_ which is used to run the bytecode file.

```sh

build/pclc --help
# Allowed options:
#  -h, --help        Print this help message
#  -a, --ast-dump    Dump AST
#  -i, --input arg   Specify input file
#  -o, --output arg  Specify output file for compiled program
#  -d, --disas       Disassemble generated code (does not run the program)

# Example usage:
build/pclc -i examples/scan.pcl

# Or alternatively: 
build/pclc -i examples/fib.pcl -o a.out
build/pclvm -i a.out

# To dump the disassembled code:
build/pclc -i examples/print_read.pcl -d
# .constant_pool
# 0x00000000 = { 0 }

# .code
# 0x00000000 push_const [ 0x00000000 ]
# 0x00000005 push_read
# 0x00000006 mov_local [ 0x00000000 ]
# 0x0000000b push_local [ 0x00000000 ]
# 0x00000010 print
# 0x00000011 pop
# 0x00000012 ret

# Or:
build/pcldis -i a.out
```

It is also possible to view the parse tree before semantic analysis. To do this you have to provide -a flag:

```sh
build/pclc -i examples/fib.pcl -a > fib.dump
dot -Tpng fib.dump > fib.png
```