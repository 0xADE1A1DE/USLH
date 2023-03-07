#!/bin/bash

folder=~/Desktop/llvm-project/build/bin/

### Compile with default slh
$folder/clang main.c -mspeculative-load-hardening -o leak


### Compile with branch condition hardening
$folder/clang main.c -mspeculative-load-hardening -mllvm -x86-slh-sbhAll -o fix
