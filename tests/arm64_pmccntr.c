#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

static inline uint64_t read_pmccntr(void)
{
	uint64_t val;
	asm volatile("mrs %0, pmccntr_el0" : "=r"(val));
	return val;
}

int main() {
uint64_t val, ret, start, stop;
char *test;
// Disable cycle counter overflow
asm volatile("msr pmintenset_el1, %0" : : "r" ((uint64_t)(0 << 31)));
// Disable cycle counter
asm volatile("msr pmcntenset_el0, %0" :: "r" (0 << 31));
// Enable cycle counter
asm volatile("msr pmcntenset_el0, %0" :: "r" (1 << 31));
// Enable user-mode access to counter
asm volatile("msr pmuserenr_el0, %0" :: "r" ((1 << 0) | (1 << 2)));
// Clear counter and start
asm volatile("mrs %0, pmcr_el0" : "=r" (val));
	val |= ((1 << 0) | (1 << 2));
	asm volatile("isb");
	asm volatile("msr pmcr_el0, %0" :: "r" (val));
	val = (1 << 27);
	asm volatile("msr pmccfiltr_el0, %0" :: "r" (val));   
start = read_pmccntr();
sleep(1);
stop = read_pmccntr();
printf("Start: %llu, Stop: %llu, diff: %llu\n\n", start, stop, stop - start);
}
