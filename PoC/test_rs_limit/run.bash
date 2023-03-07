#!/bin/bash

## Set to val to 0 to test fast_value or set val to 1 to test slow_value
val=1
taskset -c 1 ./a.out $val > tmp.txt
