/*
ObjectPool by Dadao Inc.
1. resources in disposed shared pointers will be stored in the free pool
2. new allocations may be direct fetch from the free pool if available.
otherwise the new expression is called.
�����, ʵ����Դ�Ļ�������
1. �������ٵĹ���ָ���е���Դ��������
2. �����������Դӻ���վ��ֱ�ӷ�����Դ, ����ͨ��new��������Դ
*/

#ifndef __DDRGENERAL_OBJECT_POOL_H_INCLUDED__
#define __DDRGENERAL_OBJECT_POOL_H_INCLUDED__

#include <mutex>
#include "AdaptiveDQ.hpp"

namespace DDRGeneral {

template <typename T> class ObjectPool
{
public:
	/*
	Set buffer size of the recycling pool (in number of object copies)
	���û��ջ�������С(������Դ�ķ���)
	*/
	static void SetPoolCap(int nMaxPoolInstances) {
		getIns()->setCap(nMaxPoolInstances);
	}
	/*
	Allocate resource. Note that recycles resource will NOT be re-initialized or
	assigned, so its caller's duty to handle these operations.
	������Դ. ע��, ʹ�û�����Դʱ����������κγ�ʼ���͸�ֵ����(��Դ����ԭֵ), 
	������Ӧ�ֶ��������ϲ���
	*/
	static std::shared_ptr<T> Allocate() {
		return getIns()->Allocate();
	}
	/*
	Clone objects by allocating and ASSIGN resources.
	��¡��Դ. ע��, ������Դ������ ��ֵ ����
	*/
	static std::shared_ptr<T> Clone(const T *pOri) {
		return getIns()->clone(pOri);
	}
	static std::shared_ptr<T> Clone(const T &ori) {
		return getIns()->clone(ori);
	}
	static std::shared_ptr<T> Clone(T &&ori) {
		return getIns()->clone(ori);
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
		std::lock_guard<std::mutex> lg(g_p->m_loc);
		m_maxNBuf = (nCap <= 1) ? 1 : nCap;
	}
	std::shared_ptr<T> allocate() {
		std::lock_guard<std::mutex> lg(m_loc);
		if (!m_poolQ.empty()) {
			allocCatch(true);
			T *ptt;
			m_poolQ.pop_back(ptt);
			return std::shared_ptr<T>(ptt, recycle);
		} else {
			allocCatch(false);
			std::shared_ptr<T> tt(new T, recycle);
			return tt;
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
	����ģʽ, ����GetIns()�����з���Ϳ�¡����
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
			std::lock_guard<std::mutex> lg(g_p->m_loc);
			if (g_p->m_poolQ.size() < g_p->m_maxNBuf) {
				g_p->m_poolQ.emplace_back(pEle);
				return;
			}
			while (g_p->m_poolQ.size() > g_p->m_maxNBuf) {
				T *ptt;
				g_p->m_poolQ.pop_back(ptt);
				delete ptt;
			}
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
	std::mutex m_loc;
	AdaptiveDequeue<T*> m_poolQ;

	static const int s_queryPeriod = 40;
	bool m_bTesting;
	int m_nAllocReq, m_nAllocHit;
};

template <typename T>
ObjectPool<T>* ObjectPool<T>::g_p = nullptr;

}

#endif // __DDRGENERAL_OBJECT_POOL_H_INCLUDED__
