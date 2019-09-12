/*
ObjectPool by Dadao Inc.
1. resources in disposed shared pointers will be stored in the free pool
2. new allocations may be direct fetch from the free pool if available.
otherwise the new expression is called.
对象池, 实现资源的回收利用
1. 即将销毁的共享指针中的资源将被回收
2. 分配器将尝试从回收站中直接分配资源。如果成功则试图调用该类型的Reset()函数重
置资源(Reset()不存在则无操作); 否则通过new建立新资源
*/

#ifndef __DDRGENERAL_OBJECT_POOL_H_INCLUDED__
#define __DDRGENERAL_OBJECT_POOL_H_INCLUDED__

#include <atomic>
#include "../MTLib/AtomicLock.hpp"
#include "DDR_Traits.hpp"
#include "AdaptiveDQ.hpp"

namespace DDRGeneral {
;
template <typename T> class ObjectPool
{
public:
	/*
	如果T类型具有Reset()成员函数, 则从回收站分配资源时会先执行Reset()进行资源重置
	*/
	CREATE_MEMBER_CHECK(Reset);

	/*
	Set buffer size of the recycling pool (in number of object copies)
	设置回收缓冲区大小(对象资源的份数)
	*/
	static void SetPoolCap(int nMaxPoolInstances)
	{
		getIns()->setCap(nMaxPoolInstances);
	}
	/*
	Allocate resource. Recycled resource will call Reset() if available
	分配资源. 使用回收资源时会试图调用T::Reset()重置资源
	*/
	static std::shared_ptr<T> Allocate()
	{
		return getIns()->allocate();
	}
	/*
	Clone objects by allocating and ASSIGN resources.
	克隆资源. 注意, 分配资源后会进行 赋值 操作
	*/
	static std::shared_ptr<T> Clone(const T *pOri)
	{
		return getIns()->clone(pOri);
	}
	static std::shared_ptr<T> Clone(const T &ori)
	{
		return getIns()->clone(ori);
	}
	static std::shared_ptr<T> Clone(T &&ori)
	{
		return getIns()->clone(std::move(ori));
	}

protected:
	ObjectPool() : m_maxNBuf(10), m_nAllocReq(0), m_nAllocHit(0), m_bTesting(true) {}
	virtual ~ObjectPool()
	{
		while (!m_poolQ.empty()) {
			T *ptt;
			m_poolQ.pop_back(ptt);
			delete ptt;
		}
	}
	void setCap(int nCap)
	{
		m_loc.lock();
		m_maxNBuf = (nCap <= 1) ? 1 : nCap;
		m_loc.unlock();
	}
	std::shared_ptr<T> allocate()
	{
		m_loc.lock();
		if (!m_poolQ.empty()) {
			allocCatch(true);
			T *ptt;
			m_poolQ.pop_back(ptt);
			m_loc.unlock();
			Reset(ptt);
			return std::shared_ptr<T>(ptt, recycle);
		} else {
			allocCatch(false);
			m_loc.unlock();
			return std::shared_ptr<T>(new T, recycle);
		}
	}
	std::shared_ptr<T> clone(const T *pOri)
	{
		auto tt = allocate();
		*(tt.get()) = *pOri;
		return tt;
	}
	std::shared_ptr<T> clone(const T &ori)
	{
		auto tt = allocate();
		*(tt.get()) = ori;
		return tt;
	}
	std::shared_ptr<T> clone(T &&ori)
	{
		auto tt = allocate();
		*(tt.get()) = ori;
		return tt;
	}
	void allocCatch(bool bHit)
	{
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
	static ObjectPool* getIns()
	{
		if (g_p) {
			return g_p;
		} else {
			g_p = new ObjectPool<T>();
			atexit(destroy);
			return g_p;
		}
	}
	static void recycle(T *pEle)
	{
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
	static void destroy()
	{
		if (g_p) {
			delete g_p;
			g_p = nullptr;
		}
	}
	template <typename T> static
	typename std::enable_if_t<HAS_MEMBER(T, Reset), void> Reset(T *pObj)
	{
		pObj->Reset();
	}
	template <typename T> static
	typename std::enable_if_t<!HAS_MEMBER(T, Reset)> Reset(T *pObj) {}

private:
	static ObjectPool *g_p;
	int m_maxNBuf;
	DDRMTLib::AtomicLock<true> m_loc;
	AdaptiveDequeue<T*> m_poolQ;

	static const int s_queryPeriod = 40;
	bool m_bTesting;
	int m_nAllocReq, m_nAllocHit;
};

template <typename T>
ObjectPool<T>* ObjectPool<T>::g_p = nullptr;

}

#endif // __DDRGENERAL_OBJECT_POOL_H_INCLUDED__
