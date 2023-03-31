# Ultimate SLH: Taking Speculative Load Hardening to the Next Level
*Authors:* Zhiyuan Zhang, Gilles Barthe, Chitchanok Chuengsatiansup, Peter Schwabe, Yuval Yarom.


*The paper is accepted in USENIX Security 2023 Fall Round*
(A preprint verstion of the paper is available at [paper](https://cs.adelaide.edu.au/~yval/pdfs/ZhangBCSY22.pdf))


## Ultimate SLH

### Experiment Environment
The experiments have been tested on 8th(i5-8265U), 9th (i7-9750H), 10th(i7-10710U) Gen Intel Processors running Ubuntu 20.04.  
To compile the customized LLVM, you will need at least 8GB RAM.

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
Please set processor governor to *performance* to get stable result for evaluating the artifact.  
You can achieve this with *cpupower* command.

### General Setting
Please set the processor governor to performance for evaluating the artifact.  


### USLH: To fix SLH
We provide code for USLH under folder *\LLVM_FIX*. Please follow the instructions on README to compile it. It may take a while to compile ;) (On i7-10710U, 4.1GHz, it takes 40 minutes)

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

### Gadget Searching
In the modified LLVM code we provide, we also include an llvm backend pass that performs static analysis on the source code during the compilarion. Please refer to folder *LLVM_FIX* for more information.
