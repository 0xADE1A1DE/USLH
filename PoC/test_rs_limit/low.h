#if 1
static inline uint64_t rdtscp()
{
   uint32_t a, d;
   __asm__ volatile("rdtscp" : "=a" (a), "=d" (d));
   return ((uint64_t)a) | (((uint64_t)d) << 32);
}

#else

static inline uint32_t rdtscp() {
  uint32_t rv;
  asm volatile ("rdtscp": "=a" (rv) :: "edx", "ecx");
  return rv;
}
#endif

static inline uint32_t memaccesstime(void *v) {
  uint32_t rv;
  asm volatile (
      "mfence\n"
      "lfence\n"
      "rdtscp\n"
      "mov %%eax, %%esi\n"
      "mov (%1), %%eax\n"
      "rdtscp\n"
      "sub %%esi, %%eax\n"
      : "=&a" (rv): "r" (v): "ecx", "edx", "esi");
  return rv;
}

void delayloop(int cycles)
{
  uint64_t start = rdtscp();
  while ((rdtscp()-start) < cycles);
}

long long read_pmc(unsigned long a)
{
  unsigned int lo = 0, hi = 0;
  asm volatile (
		"rdpmc"
		: "=a"(lo), "=d"(hi)
		: "c"(a));
  return ((uint64_t) hi << 32) | lo;
}

#define CLFLUSH(a) \
{ \
      asm volatile ("clflush 0(%0)": : "r" (&a):); \
}


#define ACCESS_MEMORY(a,b) \
{ \
  asm volatile ("mov (%1), %0" : "+r"(a) : "r"(&b)); \
}

#define INCREASE_R13  asm volatile ("inc %ecx\n");

#define INCREASE1 INCREASE_R13
#define INCREASE2 INCREASE1 INCREASE1
#define INCREASE4 INCREASE2 INCREASE2
#define INCREASE8 INCREASE4 INCREASE4
#define INCREASE16 INCREASE8 INCREASE8
#define INCREASE32 INCREASE16 INCREASE16
#define INCREASE64 INCREASE32 INCREASE32
#define INCREASE128 INCREASE64 INCREASE64
#define INCREASE256 INCREASE128 INCREASE128
#define INCREASE512 INCREASE256 INCREASE256

#define ACCESS1(a,b)  ACCESS_MEMORY(a,b)
#define ACCESS2(a,b)  ACCESS1(a,b)  ACCESS1(a,b)
#define ACCESS4(a,b)  ACCESS2(a,b)  ACCESS2(a,b)
#define ACCESS8(a,b)  ACCESS4(a,b)  ACCESS4(a,b)
#define ACCESS16(a,b) ACCESS8(a,b)  ACCESS8(a,b)
#define ACCESS32(a,b) ACCESS16(a,b) ACCESS16(a,b)
#define ACCESS64(a,b) ACCESS32(a,b) ACCESS32(a,b)
#define ACCESS128(a,b)	ACCESS64(a,b) ACCESS64(a,b)
#define ACCESS256(a,b)	ACCESS128(a,b)	ACCESS128(a,b)
#define ACCESS512(a,b)	ACCESS256(a,b)	ACCESS256(a,b)

#define NOP "nop;"
#define NOP1 NOP
#define NOP2 NOP1 NOP1
#define NOP4 NOP2 NOP2
#define NOP8 NOP4 NOP4
#define NOP16 NOP8 NOP8
#define NOP32 NOP16 NOP16
#define NOP64 NOP32 NOP32
#define NOP128 NOP64 NOP64
#define NOP256 NOP128 NOP128
#define NOP512 NOP256 NOP256
#define NOP1024 NOP512 NOP512
#define NOP2048 NOP1024 NOP1024
#define NOP4096 NOP2048 NOP2048
#define NOP224	NOP128 NOP64 NOP32

#if 0
#define NOP	      asm volatile ("nop");
#define NOP1 NOP
#define NOP2 NOP1 NOP1
#define NOP4 NOP2 NOP2
#define NOP8 NOP4 NOP4
#define NOP16 NOP8 NOP8
#define NOP32 NOP16 NOP16
#define NOP64 NOP32 NOP32
#define NOP128 NOP64 NOP64
#define NOP256 NOP128 NOP128
#define NOP512 NOP256 NOP256
#define NOP1024 NOP512 NOP512
#define NOP2048 NOP1024 NOP1024
#define NOP4096 NOP2048 NOP2048
#endif

#define BARRIER asm volatile ("lfence;\nmfence;\nsfence");


#define DIVIDE(a,b) asm volatile ("movsd %0, %%xmm0;\nmovsd %1, %%xmm1;\ndivsd %%xmm1, %%xmm0"::"x"(a),"x"(b));
#define DIVIDE1(a,b) DIVIDE(a,b)
#define DIVIDE2(a,b) DIVIDE1(a,b) DIVIDE1(a,b)
#define DIVIDE4(a,b) DIVIDE2(a,b) DIVIDE2(a,b)
#define DIVIDE8(a,b) DIVIDE4(a,b) DIVIDE4(a,b)
#define DIVIDE16(a,b) DIVIDE8(a,b) DIVIDE8(a,b)
#define DIVIDE32(a,b) DIVIDE16(a,b) DIVIDE16(a,b)
#define DIVIDE64(a,b) DIVIDE32(a,b) DIVIDE32(a,b)
#define DIVIDE128(a,b) DIVIDE64(a,b) DIVIDE64(a,b)
#define DIVIDE256(a,b) DIVIDE128(a,b) DIVIDE128(a,b)
#define DIVIDE512(a,b) DIVIDE256(a,b) DIVIDE256(a,b)
#define DIVIDE1024(a,b) DIVIDE512(a,b) DIVIDE512(a,b)
#define DIVIDE2048(a,b) DIVIDE1024(a,b) DIVIDE1024(a,b)
#define DIVIDE4096(a,b) DIVIDE2048(a,b) DIVIDE2048(a,b)







