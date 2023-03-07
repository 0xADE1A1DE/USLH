#!/bin/bash

folder=~/Desktop/llvm-project/build/bin 

$folder/clang main.c -O1 -S -emit-llvm
$folder/llc main.ll --x86-speculative-load-hardening --x86-slh-vtInstr=true
clang main.s

v=$[ $RANDOM % 256 ]
for i in {0..20}
do
  taskset -c 1 ./a.out $v
  sleep 0.1
done
