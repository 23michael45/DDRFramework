/*
ObjectPool by Dadao Inc.
1. resources in disposed shared pointers will be stored in the free pool
2. new allocations may be direct fetch from the free pool if available.
otherwise the new expression is called.
对象池, 实现资源的回收利用
1. 即将销毁的共享指针中的资源将被回收
2. 分配器将尝试从回收站中直接分配资源, 否则通过new建立新资源
*/

#ifndef __DDRGENERAL_OBJECT_POOL_H_INCLUDED__
#define __DDRGENERAL_OBJECT_POOL_H_INCLUDED__

#include <atomic>
#include "AdaptiveDQ.hpp"

namespace DDRGeneral {

class AtomicLock
{
public:
	AtomicLock() : m_flag(false) {}
	void lock() {
		while (m_flag.exchange(true, std::memory_order_relaxed));
		std::atomic_thread_fence(std::memory_order_acquire);
	}
	void unlock() {
		std::atomic_thread_fence(std::memory_order_release);
		m_flag.store(false, std::memory_order_relaxed);
	}
private:
	std::atomic<bool> m_flag;
};

template <typename T> class ObjectPool
{
public:
	/*
	Set buffer size of the recycling pool (in number of object copies)
	设置回收缓冲区大小(对象资源的份数)
	*/
	static void SetPoolCap(int nMaxPoolInstances) {
		getIns()->setCap(nMaxPoolInstances);
	}
	/*
	Allocate resource. Note that recycles resource will NOT be re-initialized or
	assigned, so its caller's duty to handle these operations.
	分配资源. 注意, 使用回收资源时不会对其做任何初始化和赋值操作(资源保持原值), 
	调用者应手动进行以上操作
	*/
	static std::shared_ptr<T> Allocate() {
		return getIns()->allocate();
	}
	/*
	Clone objects by allocating and ASSIGN resources.
	克隆资源. 注意, 分配资源后会进行 赋值 操作
	*/
	static std::shared_ptr<T> Clone(const T *pOri) {
		return getIns()->clone(pOri);
	}
	static std::shared_ptr<T> Clone(const T &ori) {
		return getIns()->clone(ori);
	}
	static std::shared_ptr<T> Clone(T &&ori) {
		return getIns()->clone(std::move(ori));
	}

protected:
	ObjectPool() : m_maxNBuf(10), m_nAllocReq(0), m_nAllocHit(0), m_bTesting(true) {}
	virtual ~ObjectPool() {
		while (!m_poolQ.empty()) {
			T *ptt;
			m_poolQ.pop_back(ptt);
			delete ptt;
		}
	}
	void setCap(int nCap) {
		m_loc.lock();
		m_maxNBuf = (nCap <= 1) ? 1 : nCap;
		m_loc.unlock();
	}
	std::shared_ptr<T> allocate() {
		m_loc.lock();
		if (!m_poolQ.empty()) {
			allocCatch(true);
			T *ptt;
			m_poolQ.pop_back(ptt);
			m_loc.unlock();
			return std::shared_ptr<T>(ptt, recycle);
		} else {
			allocCatch(false);
			m_loc.unlock();
			return std::shared_ptr<T>(new T, recycle);
		}
	}
	std::shared_ptr<T> clone(const T *pOri) {
		auto tt = allocate();
		*(tt.get()) = *pOri;
		return tt;
	}
	std::shared_ptr<T> clone(const T &ori) {
		auto tt = allocate();
		*(tt.get()) = ori;
		return tt;
	}
	std::shared_ptr<T> clone(T &&ori) {
		auto tt = allocate();
		*(tt.get()) = ori;
		return tt;
	}
	void allocCatch(bool bHit) {
		if (!m_bTesting) {
			if (bHit) {
				++m_nAllocHit;
			}
			if (++m_nAllocReq >= s_queryPeriod) {
				float probHit = (m_nAllocHit + 0.0f) / m_nAllocReq;
				float capAdj = 2.0f - probHit * 1.1f;
				m_maxNBuf = (int)(m_maxNBuf * capAdj + 0.5f);
				if (m_maxNBuf < 1) {
					m_maxNBuf = 1;
				}
				m_nAllocReq = m_nAllocHit = 0;
			}
		} else if (++m_nAllocReq >= s_queryPeriod) {
			m_nAllocReq = m_nAllocHit = 0;
			m_bTesting = false;
		}
	}
	/*
	Single global instance so call GetIns() to proceed.
	单例模式, 调用GetIns()来进行分配和克隆操作
	*/
	static ObjectPool* getIns() {
		if (g_p) {
			return g_p;
		} else {
			g_p = new ObjectPool<T>();
			atexit(destroy);
			return g_p;
		}
	}
	static void recycle(T *pEle) {
		if (g_p) {
			g_p->m_loc.lock();
			if (g_p->m_poolQ.size() < g_p->m_maxNBuf) {
				g_p->m_poolQ.emplace_back(pEle);
				g_p->m_loc.unlock();
				return;
			}
			while (g_p->m_poolQ.size() > g_p->m_maxNBuf) {
				T *ptt;
				g_p->m_poolQ.pop_back(ptt);
				g_p->m_loc.unlock();
				delete ptt;
				g_p->m_loc.lock();
			}
			g_p->m_loc.unlock();
		}
		delete pEle;
	}
	static void destroy() {
		if (g_p) {
			delete g_p;
			g_p = nullptr;
		}
	}

private:
	static ObjectPool *g_p;
	int m_maxNBuf;
	AtomicLock m_loc;
	AdaptiveDequeue<T*> m_poolQ;

	static const int s_queryPeriod = 40;
	bool m_bTesting;
	int m_nAllocReq, m_nAllocHit;
};

template <typename T>
ObjectPool<T>* ObjectPool<T>::g_p = nullptr;

}

#endif // __DDRGENERAL_OBJECT_POOL_H_INCLUDED__
