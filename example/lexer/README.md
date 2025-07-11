

## Program

This example program tokenizes the input system-verilog source code (placed in `foo.sv`),
and then prints the tokens.


## Build


```shell
# generate ninja build files
$ modi
# build program using ninja
$ ninja -f build/build.ninja

# run program
$ build/lex
