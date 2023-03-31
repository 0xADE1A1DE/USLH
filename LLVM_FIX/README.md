# Fixing Speculative Load Hardening

Based on the original SLH, we harden following things:
* Flag setting instructions that followed by a conditional control flow transfer
* The loads from fixed addresses (e.g. load from stack)
* Store addresses
* Variable timing instructions


## Flag setting instructions
We harden operands of flag setting instructions that set flags for control flow transfers.  
Based on the mode of SLH, aSLH or vSLH (please refer to our paper for more detail), we provide two modes on hardening flag setting instructions:  
* In aSLH, we harden the address of operands. We skip those that are already hardened by original SLH logic and we harden memory accesses from stack.
* In vSLH, we unfold the flag setting instruciton, which reads from memory and store values into a variable, harden the loaded value and put it back to flag setting instructions.

Besides, we harden registers that are not hardened by original SLH logic, which indicates that these registers hold value that is loaded in a safe status.

## Loads from fixed addresses
We harden physical registers RSP, RBP at the beginning of each basic block.

## Store addresses
Similar to hardening addresses of memory reading, except that we are hardening addresses of memory writing.

## Variable timing instructions
We harden floating point instructions by poisoning their operands and for X87 instructions, we insert LFENCE.  
We also harden rcx register for REPEAT instructions. They are special cases for conditional branching instructions.


**Code is in file X86SpeculativeLoadHardening.cpp**

### Patch
We provide a patch to the latest github version of SLH (https://github.com/llvm/llvm-project/tree/4b1b9e22b3cb854e90e718e9d10d7ceb6e12f26a) 

# Find Gadget
The code is in file X86MIRanalyze.cpp.

## How to use it
After the clang is compiled, you can use the backend pass with command  
`$path_to_binary/clang main.c -mllvm -x86-mir-analyze`

Clang will print ``Found it --> `` if it finds a gadget.


## Compile several softwares
We show the commands to compile softwares in Table 2 with our analysis pass.

### OpenSSL 1.1.1q
1. `./config CC=path/to/clang CXX=path/to/clang++ CFLAGS="-Wall -O3 -mllvm -x86-mir-analyze" CXXFLAGS="-Wall -O3 -mllvm -x86-mir-analyze"`  
2. `make`

### bash-5.1.16
1. `./configure CC=path/to/clang CFLAGS='-O3 -mllvm -x86-mir-analyze'`  
2. `make`


### libgcrypto 1.8.9
1. `./configure CC=path/to/clang CFLAGS="g -O1 -fvisibility=hidden -fno-delete-null-pointer-checks -Wall -mllvm -x86-mir-analyze"`
2. `make`

### musl 1.2.3
1. `./configure CC=path/to/clang CFLAGS="-mllvm -x86-mir-analyze"`
2. `make`

### python 3.9.14 
1. './configure CC=path/to/clang CXX=path/to/clang++ CFLAGS="-mllvm -x86-mir-analyze"'
2. `make`

# Compile USLH

USLH is developedn upon the LLVM git repo. We provide our modified version at https://doi.org/10.5281/zenodo.7704637

To build the cusomized compiler, you can follow the instructions at https://clang.llvm.org/get_started.html.  

In summary:
1. `cd llvm-project` (Go to the directory that holds the code)
2. `mkdir build && cd build`
3. `cmake -DLLVM_ENABLE_PROJECTS=clang -G "Unix Makefiles" -DLLVM_ENABLE_ASSERTIONS=On -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_LINKER=gold ../llvm`
4. `make -j10` (Note that if you have limited RAM, you need to use fewer cores to build the project)
5. After the compilation, you should find *clang* under folder /build/bin/




