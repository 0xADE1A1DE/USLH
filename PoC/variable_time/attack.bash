#!/bin/bash

~/Desktop/llvm-project/build/bin/clang main.c -O1 -S -emit-llvm
~/Desktop/llvm-project/build/bin/llc main.ll
clang main.s

v=$[ $RANDOM % 256 ]
for i in {0..20}
do
  taskset -c 1 ./a.out $v
  sleep 0.1
done
