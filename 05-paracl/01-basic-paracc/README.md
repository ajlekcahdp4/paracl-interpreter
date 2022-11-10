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
#   -h, --help        Print this help message
#   -i, --input arg   Specify input file
#   -o, --output arg  Specify output file for compiled program
#   -d, --disas       Disassemble generated code (does not run the program)

# Example usage:
build/pclc -i test_programs/scan.pcl

# Or alternatively: 
build/pclc -i test_programs/fib.pcl -o a.out
build/pclvm -i a.out

# To dump the disassembled code:
build/pclc -i test_programs/print_read.pcl -d
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