#ifndef __DDRMTLIB_GENERAL_LOCK_GUARD_H_INCLUDED__
#define __DDRMTLIB_GENERAL_LOCK_GUARD_H_INCLUDED__

#include <mutex>
#include <shared_mutex>
#include "AtomicLock.hpp"

namespace DDRMTLib {

class _lock_guard {
public:
	_lock_guard(std::timed_mutex &lll, int waitTimeMilSec = -1) : m_type(0)
	{
		lock(lll, waitTimeMilSec);
	}
	_lock_guard(std::mutex &lll) : m_type(0)
	{
		lock(lll);
	}
#if ((defined(__GNUC__) || defined(__GNUG__)) && __cplusplus >= 201703L) || (defined(_MSC_VER) && __cplusplus >= 199711L)
	_lock_guard(bool bRead, std::shared_mutex &lll) : m_type(0)
	{
		lock(bRead, lll);
	}
#endif
	_lock_guard(bool bRead, std::shared_timed_mutex &lll, int waitTimeMilSec = -1) : m_type(0)
	{
		lock(bRead, lll, waitTimeMilSec);
	}
	template <bool bYield>
	_lock_guard(AtomicLock<bYield> &lll, bool bTryOnlyOnce = false) : m_type()
	{
		lock(lll, bTryOnlyOnce);
	}

	bool lock(std::mutex &lll)
	{
		unlock();
		m_p = &lll;
		lll.lock();
		m_type = 1;
		return (bool)(*this);
	}
	bool lock(std::timed_mutex &lll, int waitTimeMilSec)
	{
		unlock();
		m_p = &lll;
		if (waitTimeMilSec >= 0) {
			m_type = lll.try_lock_for(std::chrono::milliseconds(waitTimeMilSec)) ? 2 : 0;
		} else {
			lll.lock();
			m_type = 2;
		}
		return (bool)(*this);
	}
#if ((defined(__GNUC__) || defined(__GNUG__)) && __cplusplus >= 201703L) || (defined(_MSC_VER) && __cplusplus >= 199711L)
	bool lock(bool bRead, std::shared_mutex &lll)
	{
		unlock();
		m_p = &lll;
		if (bRead) {
			lll.lock_shared();
		} else {
			lll.lock();
		}
		m_bRead = bRead;
		m_type = 3;
		return (bool)(*this);
	}
#endif
	bool lock(bool bRead, std::shared_timed_mutex &lll, int waitTimeMilSec)
	{
		unlock();
		m_p = &lll;
		m_bRead = bRead;
		if (bRead) {
			if (waitTimeMilSec >= 0) {
				m_type = lll.try_lock_shared_for(std::chrono::milliseconds(waitTimeMilSec)) ? 4 : 0;
			} else {
				lll.lock_shared();
				m_type = 4;
			}
		} else {
			if (waitTimeMilSec >= 0) {
				m_type = lll.try_lock_for(std::chrono::milliseconds(waitTimeMilSec)) ? 4 : 0;
			} else {
				lll.lock();
				m_type = 4;
			}
		}
		return (bool)(*this);
	}
	template <bool bYield>
	bool lock(AtomicLock<bYield> &lll, bool bTryOnlyOnce)
	{
		unlock();
		bool bSucc = true;
		if (!bTryOnlyOnce) {
			lll.lock();
		} else if (!lll.try2lock()) {
			bSucc = false;
		}
		if (bSucc) {
			m_p = &lll;
			m_type = 5;
			m_bRead = bYield;
		}
		return (bool)(*this);
	}
	operator bool()
	{
		return (0 != m_type);
	}
	void unlock()
	{
		switch (m_type) {
		case 1:
			((std::mutex*)m_p)->unlock();
			break;
		case 2:
			((std::timed_mutex*)m_p)->unlock();
			break;
#if ((defined(__GNUC__) || defined(__GNUG__)) && __cplusplus >= 201703L) || (defined(_MSC_VER) && __cplusplus >= 199711L)
		case 3:
			if (m_bRead) {
				((std::shared_mutex*)m_p)->unlock_shared();
			} else {
				((std::shared_mutex*)m_p)->unlock();
			}
			break;
#endif
		case 4:
			if (m_bRead) {
				((std::shared_timed_mutex*)m_p)->unlock_shared();
			} else {
				((std::shared_timed_mutex*)m_p)->unlock();
			}
			break;
		case 5:
			if (m_bRead) {
				((AtomicLock<true>*)m_p)->unlock();
			} else {
				((AtomicLock<false>*)m_p)->unlock();
			}
			break;
		default:
			break;
		}
		m_type = 0;
	}
	~_lock_guard()
	{
		unlock();
	}
private:
	int m_type; // 0 - no lock; 1 - mutex; 2 - timed mutex; 3 - shared mutex; 4 - shared timed mutex
	void *m_p;
	bool m_bRead; // read or write mode (shared/exclusive) for shared mutex, bYield for atomicSpinLock
};

}

#endif // __DDRMTLIB_GENERAL_LOCK_GUARD_H_INCLUDED__

