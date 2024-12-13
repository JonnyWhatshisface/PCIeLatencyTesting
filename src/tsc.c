#include <linux/time.h>
#include <linux/rtc.h>

#include "tsc.h"

static inline uint64_t read_pmccntr(void)
{
	uint64_t val;
	asm volatile("mrs %0, pmccntr_el0" : "=r"(val));
	return val;
}


int check_tsc_invariant(void) {

#ifdef __aarch64__

	uint64_t val, start, stop;

	ktime_t stime, etime;
	s64 actual_time;

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

	preempt_disable();
	start = read_pmccntr();
	stime = ktime_get();
	pr_info("Print test...\n");
	stop = read_pmccntr();
	etime = ktime_get();
	preempt_enable();
	actual_time = ktime_to_ns(ktime_sub(etime, stime));
	pr_info("Start: %llu, Stop: %llu, diff: %llu\n\n", start, stop, stop - start);
	pr_info("Elapsed time: %lld\n", (long long) actual_time);

	return 0;

#elif __x86_64__
	
	uint32_t edx;

	/* Check for RDTSCP instruction */
	asm volatile("cpuid"
		     : "=d" (edx)
		     : "a" (0x80000001)
		     : "rbx", "rcx"
		);

	if (edx | 0x8000000) {
		pr_info(DRIVER_NAME ": CPUID.80000001:EDX[bit 27] == 1, "
			"RDTSCP instruction available\n");
	}
	else {
		pr_info(DRIVER_NAME ": CPUID.80000001:EDX[bit 27] == 0, "
			"RDTSCP instruction not available\n"
			"Exiting here\n");
		return 0;
	}

	/* Check for TSC invariant bit */
	asm volatile("cpuid"
		     : "=d" (edx)
		     : "a" (0x80000007)
		     : "rbx", "rcx"
		);

	if (edx | 0x100) {
		pr_info(DRIVER_NAME ": CPUID.80000007:EDX[bit 8] == 1, "
			"TSC is invariant\n");
		return 1;
	}
	else {
		pr_info(DRIVER_NAME ": CPUID.80000007:EDX[bit 8] == 0, "
			"TSC is not invariant\n"
			"Exiting here\n");
		return 0;
	}

#endif

	return 0;
}