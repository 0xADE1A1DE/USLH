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
