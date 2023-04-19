# Leakages from variable timing instructions
This PoC is for evaluating leakages from variable timing instructions (sqrtsd).  
We provide the implementation of Listing 6 in the paper.  

## Code
The prorgam reads a randomly generated secret and feed it into function *victim_function*.  
The function selects the fast value or slow value with a constant time implementation.  
The selected value is then fed into a sequence of `sqrtsd and mulsd` operations bit-by-bit.  
As shown in the paper, following the variable timing instructions 
is a memory access to a public address.  

## Compile & Run
Simply execute `$bash attack.bash` or `$bash mitigate.bash` could compile and evaluate the PoC 
without or with mitigation.

### Issues
(Thanks comments from anonymous reviewers of USENIX Security AE)  
You may see following issues when compile the program:  
`/usr/bin/ld: /tmp/main-19d661.o: relocation R_X86_64_32 against '.rodata.str1.1' can not be used when making a PIE object; recompile with -fPIE
/usr/bin/ld: failed to set dynamic section sizes: bad value
clang: error: linker command failed with exit code 1 (use -v to see invocation)`

To solve the issue, you will need to add -no-pie to the second clang call in *attack.bash* and *mitigate.bash* (clang -no-pie main.s).

## Notes
- The number of pairs of `sqrtsd, mulsd` is various from platform to platform. 
You will need to find a vulnerable pair on your machine.  
To test different pairs of operations, you can change the repeat times at line 36 of main.c.  
For example `asm volatile (".rept 10;\nsqrtsd %xmm0, %xmm0;\nmulsd %xmm0, %xmm0;\n.endr");` 
to play with 10 pairs of the operations.

You can also find a vulnerable pair of operqtions in folder "../test_rs_limit".  
The code under the folder can automatically run different pairs of variable timing operations.

- Since the number of vulnerable pairs of `sqrtsd, mulsd` are various from processors.  
We use inline assemblyline to adjust the number easily.  
For mitigation to work, it relies on the fact that `volatile double tmp2 = tmp * tmp` 
is compiled to store tmp2 into %xmm0.  
If the compiler does not store tmp2 to %xmm0, you may need to check the main.s to find which %xmm register is used and change the %xmm register in the inline assemblyline accrodingly.

## Expected Result
- Attack  
You can run the attack with command `$bash attack.bash`.  
The expected output should be similar to the one below:  
93  91  11  91  13  14  13  89 Guess : 116, --> 1  
The first eight values are the access time to *global_variable*. The guess value is the guessed value based on eight timing results. If the guess is correct, after `-->` shows 1, otherwise 0.  
We provide a sample output under folder /results

- Mitigate
You can test the mitigation with command `bash mitigate.bash`.
The expected output should be similar to the one below:  
90  89  91  87  90  90  90  89 Guess :   0, --> 0  
Due to the fact that all variable timing instructions are delayed until the resolve of condition, 
the *global_variable* should not be sent to the RS and not be executed.


