The code under this repo is to demonstrate that secret can be leaked from branch conditions and how to fix it.


## Code
The function *test1* contains a secret-dependent branch at line 23.  
If the secret is 1, then *global_variable1* will be accessed, otherwise, *global_variable2* will be accessed.  

In the training stage, we train the branch at line 22 (test if public) to be true and 
we train the secret relevant branch at 23 to also be true.  

During the attack, the *test1* processes a secret bit (*secret*). The bit can be either 1 or 0.  
An attacker is monitoring on the cache status of *global_variable2*.  
Note that during the train stage, *global_variable1* is always trained to be accessed. 
So the speculative execution will always fetch this memory into the cache.  
Consequently, *global_variable2* will be speculatively accessed if the secret bit is 0 and 
it will not be accessed if the secret value is 0.

## Compile 
Run `$ bash compile.bash` to compile the program to two versions: 
- *leak*: The one that has vulnerability
- *fix*: The one that has been mitigated

**You will need to change the path to the compiler with you own path.**

## Run
Execute `$./leak $val` or `$./fix $val` to play with the vulnerable version and the fixed version.  

$val indicates the value of secret, it can either be 0 or 1.

## Expected Result
### leak
When `$val` is 0, the *global_variable2* should be cached in most cases. So you will see results similar to :  
`Time results: 12`  

When `$val` is 1, the *global_variable2* should not be cached. So you will see results similar to:  
`Time results: 118`

### fix 
No matter `$val` is 1 or 0. The timing result should always indicates a cache miss on accessing *global_variable2*


## Auto Test Script
(Thanks code provided by anonymous reviewers in USENIX Security AE) 

After compiling the code, you could also run the auto_plot.py script to check the timing distribution.
