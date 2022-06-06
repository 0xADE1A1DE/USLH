# Breaking and Fixing Specluative Load Hardening

This repo contains PoC codes for paper *Breaking and Fixing Speculative Load Hardening*

In this paper, we analyze the Speculative Load Hardening, a compiler based spectre-v1 mitigation, and we propose two type of attacks that can bypass SLH mitigation.

## Speculative Load Hardening
Speculative Load Hardening (*SLH*) is a compiler based spectre-v1 mitigation and it is implemented in LLVM by Chandler Carruth.

SLH analyzes the control flow transfer and conditionally poisons the loaded value or the address of load.  
For detailed introduction of SLH, you could find it in our paper or the LLVM homepage of SLH: https://llvm.org/docs/SpeculativeLoadHardening.html .

As described by the name of SLH, it does not harden registers and secret value could still be leaked if it is stored in a register before the speculative execution.

In our work, we also demonstrate that due to the limitation of back-end resources, an access to a global variable could also leak secret value.

## PoC_1: Leakages from nested branches
Our first PoC shows that hardening memory reading is not sufficient to stop spectre-v1 attacks.  
As listed in the following code block, an attacker trains the outer branch to be mispredicted so that a secret value is speculatively processed.
```
if (security_check) {
  if (secret == 0) {
    ...
  } else {
    ...
  }
}
```
To extend the speculation window, the outer branch condition is flushed. Inner branch determines how the secret value is processed.  
Since the condition of innder branch is not flushed and it is resolved faster than the resolve of outer branch.  
Namely, in the speculation window, the execution of inner branch always reveal the value of secret.

As discussed in last section, if secret flows into a register before the speculative execution (e.g. function parameter) , SLH does not harden it. Further, we demonstrate that the contention on execution ports leaks which secret relevant path is selected.

You could find the PoC under folder Exploit_Poc/V1_SMoTherSpectre

## PoC_2: Memory access dependent on variable timing instructions
Our second PoC demonstrates that a memory access dependent on variable timing instructions can bypass vSLH (SLH that hardens loaded value). 
```
if (security_check) {
  value = sqrtsd(value);
  value = mulsd(value);
  ...
  value = sqrtsd(value);
  memory_access(address depends on value);
}
```
The execution time of floating point instructions depends on their input.
In the PoC, we utilize the fact that SQRTSD instruction has different execution time on input: 65536 (0x40f0000000000000) and 2.34e-308 (0x0010deadbeef1337).

Depends on the execution time of floating point instructions, the memory access that is dependent on them may fall out ot the sepculation window (race condition between execution time and speculation window) and thus not executed speculatively.  
An attacker could infer the input to floating point operands by checking the existence of memory in the cache.  

You could find the PoC under folder Exploit_Poc/V1_VariableTiming_dependency

## PoC_3: Memory access independent on variable timing instructions
Our thid PoC exploits the limitation of back-end limiation and demonstrate that an access to a global variable (secret independent value) leaks information.
```
if (security_check) {
  value = sqrtsd(value);
  value = mulsd(value);
  ...
  value = sqrtsd(value);
  memory_access(global variable);
}
```
Floating point execution consumes back-end resources and depends on the race condition between the free of resource and speculation window, a memory access to a global variable may or may not be performed.

You could find the PoC under folder Exploit_Poc/V1_VariableTiming_RsourceContention

We also provide a script to test the maximum floating point instructions that can be executed before the termination of speculation window.
You can find it under folder Exploit_PoC/limit_RS

