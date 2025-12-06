#ifndef FUTEX_MUTEX_H
#define FUTEX_MUTEX_H

#include <errno.h>
#include <limits.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#if defined(__x86_64__) || defined(__i386__)
#include <emmintrin.h>
#define CPU_RELAX() _mm_pause()
#elif defined(__aarch64__)
#define CPU_RELAX() __asm__ __volatile__("isb" : : : "memory")
#else
#define CPU_RELAX() ((void)0)
#endif

#define FUTEX_SPIN_LIMIT 100

/* States: 0=unlocked, 1=locked no waiters, 2=locked with waiters */
typedef atomic_uint_fast32_t futex_mutex_t;

__attribute__((unused)) static inline long
sys_futex(void *addr1, int op, int val1, const struct timespec *timeout,
	  void *addr2, int val3)
{
	return syscall(SYS_futex, addr1, op | FUTEX_PRIVATE_FLAG, val1, timeout,
		       addr2, val3);
}

__attribute__((unused)) static inline void
futex_mutex_init(futex_mutex_t *lock)
{
	atomic_init(lock, 0);
}

__attribute__((unused)) static inline void
futex_mutex_lock(futex_mutex_t *lock)
{
	uint_fast32_t c;

	c = 0;
	if (atomic_compare_exchange_strong_explicit(
		lock, &c, 1, memory_order_acquire, memory_order_relaxed)) {
		return;
	}

	for (int i = 0; i < FUTEX_SPIN_LIMIT; i++) {
		CPU_RELAX();
		if (atomic_load_explicit(lock, memory_order_relaxed) == 0) {
			c = 0;
			if (atomic_compare_exchange_weak_explicit(
				lock, &c, 1, memory_order_acquire,
				memory_order_relaxed)) {
				return;
			}
		}
	}

	if (c != 2) {
		c = atomic_exchange_explicit(lock, 2, memory_order_acquire);
	}

	while (c != 0) {
		sys_futex((void *)lock, FUTEX_WAIT, 2, NULL, NULL, 0);
		c = atomic_exchange_explicit(lock, 2, memory_order_acquire);
	}
}

__attribute__((unused)) static inline void
futex_mutex_unlock(futex_mutex_t *lock)
{
	if (atomic_fetch_sub_explicit(lock, 1, memory_order_release) == 1) {
		return;
	}

	atomic_store_explicit(lock, 0, memory_order_release);
	sys_futex((void *)lock, FUTEX_WAKE, 1, NULL, NULL, 0);
}

__attribute__((unused)) static inline int
futex_mutex_trylock(futex_mutex_t *lock)
{
	uint_fast32_t c = 0;
	if (atomic_compare_exchange_strong_explicit(
		lock, &c, 1, memory_order_acquire, memory_order_relaxed)) {
		return 0;
	}
	return EBUSY;
}

#endif
