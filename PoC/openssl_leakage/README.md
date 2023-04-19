Code under this folder test for a gadget found in openssl-1.1.1q.

# Attack openssl

## Download openssl-1.1.1q
The code is available at https://www.openssl.org/source/old/1.1.1/

## Compile Openssl
1. `$./config CC=$path_to_binary/clang CFLAGS="-mspeculative-load-hardening"`  
You should replace `$path_to_binary` with your own path to the customized clang code.

2. Compile the openssl with command `make clean && make -j12`  
The number after `-j` should be the number of logical cores you want to use to compile the project.

#### Issues on Ubuntu 22.04
(Thanks comments from anonymous reviewrs of USENIX Security AE)  
You may see issue  
`
test/v3ext.c:201:24: error: call to undeclared library function 'memcmp' 
`
when compiling OpenSSL on Ubuntu 22.04. To tackle this issue, you will need to add `#include <string.h>` to test/v3ext.c. More information is available at (https://github.com/openssl/openssl/issues/18720).

## Compile the Code
1. Before you compile the code, you should direct the path to the correct library path:  
For exmaple  
`export LD_LIBRARY_PATH=/home/zzy/Desktop/gadget_search/crypt_lib/library/openssl-1.1.1q/`  
**You should change the path to your own path to the compiled openssl library**

2 Then you can compile the code with command `cc main.c -l ssl -l crypto -Dtrain=1 -o crun`  
Check that you are using the correct path to the library with command `ldd crun`  
You should see output similar to:  
`
linux-vdso.so.1 (0x00007ffd86c63000)
        libcrypto.so.1.1 => /home/zzy/Desktop/gadget_search/crypt_lib/library/openssl-1.1.1q/libcrypto.so.1.1 (0x00007f3d5c283000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f3d5c07e000)
        libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f3d5c078000)
        libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f3d5c055000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f3d5c89f000)
`


## About `bn_mul_word`
While `a->top` is valid, the branch evaluates whether the big number is zero or not. 
If it is zero, then it invokes `BN_zero` to zero the value otherwise it performs the multiplication.

```
int BN_mul_word(BIGNUM *a, BN_ULONG w)
{
    BN_ULONG ll;

    bn_check_top(a);
    w &= BN_MASK2;
    if (a->top) {
        if (w == 0)
            BN_zero(a);
        else {
            ll = bn_mul_words(a->d, a->d, a->top, w);
            if (ll) {
                if (bn_wexpand(a, a->top + 1) == NULL)
                    return 0;
                a->d[a->top++] = ll;
            }
        }
    }
    bn_check_top(a);
    return 1;
}
```

## Run the code
You can run the code with command `./crun $bit` where `$bit` is either 1 or 0.  


## Expected Result
When the openssl is compiled with default slh, if the `$bit` is zero, the access to `BN_set_word` should be fast, as it is speculatively called by `BN_zero`. Otherwise it should be slower.


# Fix openssl
1. You should compile the openssl with command  
`$./config CC=$path_to_binary/clang CFLAGS="-mspeculative-load-hardening -mllvm -x86-slh-sbhAll"`


2. The rest steps are the same to previous descriptions.  

## Expected Result
This time, no matter the `bit` is 0 or 1, the access to `BN_set_word` should be slow.



















