# Ultimate SLH: Taking Speculative Load Hardening to the Next Level
*Authors:* Zhiyuan Zhang, Gilles Barthe, Chitchanok Chuengsatiansup, Peter Schwabe, Yuval Yarom.


*The paper is accepted in USENIX Security 2023 Fall Round*

## Ultimate SLH

### Limits of SLH
In the paper, we evaulate several limits of implemented LLVM SLH and Strong SLH.  
We firstly demonstrate that a secret value may flow into a register which is used for conditional
 control flow transfer. It can be leaked by monitoring which branch is taken from a covert channel.  
We then demonstrate that variable-timing instruction under speculative execution is also vulnerable.  
We show that the secret can be leaked from a memory access to a public fixed address which is not secret relevant.  

### Fix SLH
We fix the SLH by extending the work to match the description of SSLH and we further protect the 
variable-timing instructions.

## Artifact Evaluation
We provide PoC to demonstrate leakages from resolving branch conditions and variable timing instructions.    

### Resolving branch condition
SLH protects values loaded from the memory but not values in the register.  
We demonstrate that a nested branch can leak secret by resolving a branch speculatively.  
We further show that the the issue can be fixed by hardening the branch conditions in extra.  
You can find more information under *PoC/condition*

### Variable-time instructions
In the paper, we present a PoC of exploting variable-timem instructions: *sqrtsd*.  
The root cause of the attack is that the speculative window is restricted the Reservation Station.(RS)  
In cases that the size of Reorder Buffer (ROB) is large enough and time of solving branch condition is long enough, the limitation of RS may prevent the schedule of some operations such as memory accesses.  

In the artifact evaluation, we provide code to evaluate if the processor is vulnerable to variable-time instructions under speculative execution (See *PoC/test_rs_limit*).  
Further we provide code to evaulate a PoC attack in *PoC/variable_time*. You can evaluate both the attack and mitigation.
