#ifndef __DDRMTLIB_ATOMIC_SPINLOCK_H_INCLUDED__
#define __DDRMTLIB_ATOMIC_SPINLOCK_H_INCLUDED__

#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace DDRMTLib {

template <bool bYield = true>
class AtomicLock
{
public:
	AtomicLock() {}
	void lock();
	void unlock()
	{
		m_flag.clear(std::memory_order_release);
	}
private:
	std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};

inline void AtomicLock<true>::lock()
{
	while (m_flag.test_and_set(std::memory_order_acquire)) {
		std::this_thread::yield();
	}
}

inline void AtomicLock<false>::lock()
{
	while (m_flag.test_and_set(std::memory_order_acquire));
}

}

#endif // __DDRMTLIB_ATOMIC_SPINLOCK_H_INCLUDED__
