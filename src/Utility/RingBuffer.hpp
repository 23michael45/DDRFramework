#ifndef __DDRGENERAL_RING_BUFFER_H_INCLUDED__
#define __DDRGENERAL_RING_BUFFER_H_INCLUDED__

#include <string.h>
#include <malloc.h>

namespace DDRGeneral {
;
class RingBuffer
{
public:
	RingBuffer(int cap = 128) : m_pBuf(nullptr), m_cap(0)
	{
		adjust(cap);
	}
	bool Recap(int newCap)
	{
		return adjust(newCap);
	}
	~RingBuffer()
	{
		if (m_pBuf) {
			free(m_pBuf);
		}
	}

	bool valid() const
	{
		return (m_pBuf != nullptr);
	}
	int cap() const
	{
		return (m_pBuf ? m_mod : 0);
	}
	int size() const
	{
		return (m_pBuf ? ((m_tail + m_cap - m_head) & m_mod) : 0);
	}
	int freeSpace() const
	{
		return (m_pBuf ? (m_mod - size()) : 0);
	}

	bool Append(const void *pData, int nLen)
	{
		if (pData && nLen > 0 && m_pBuf) {
			int sz = size();
			if (nLen < m_mod) {
				int rem = m_cap - m_tail;
				if (nLen <= rem) {
					memcpy(m_pBuf + m_tail, pData, nLen);
				} else {
					memcpy(m_pBuf + m_tail, pData, rem);
					memcpy(m_pBuf, (const char*)pData + rem, nLen - rem);
				}
				sz += nLen;
				if (sz > m_mod) {
					sz = m_mod;
				}
				m_tail = (m_tail + nLen) & m_mod;
				m_head = (m_tail + m_cap - sz + m_cap) & m_mod;
			} else {
				memcpy(m_pBuf, (const char*)pData + nLen - m_mod, m_mod);
				m_head = 0;
				m_tail = m_mod;
			}
			return true;
		}
		return false;
	}

	// pTar could be null (to pop without copying)
	// nLen2pop could be negative to pop ALL
	int Pop(void *pTar, int nLen2pop)
	{
		if (0 == nLen2pop) {
			return 0;
		}
		int sz = size();
		if (nLen2pop < 0) {
			nLen2pop = sz;
		}
		if (sz > nLen2pop) {
			if (pTar) {
				int rem = m_cap - m_head;
				if (rem >= nLen2pop) {
					memcpy(pTar, m_pBuf + m_head, nLen2pop);
				} else {
					memcpy(pTar, m_pBuf + m_head, rem);
					memcpy((char*)pTar + rem, m_pBuf, nLen2pop - rem);
				}
			}
			m_head = (m_head + nLen2pop + m_cap) & m_mod;
			return nLen2pop;
		} else {
			if (pTar) {
				int rem = m_cap - m_head;
				if (rem >= sz) {
					memcpy(pTar, m_pBuf + m_head, sz);
				} else {
					memcpy(pTar, m_pBuf + m_head, rem);
					memcpy((char*)pTar + rem, m_pBuf, sz - rem);
				}
			}
			m_head = m_tail = 0;
			return sz;
		}
	}

	// nLen2move could be negative to move ALL
	int MoveTo(RingBuffer *pDstRF, int nLen2move)
	{
		if (0 == nLen2move || !pDstRF || !pDstRF->m_pBuf) {
			return 0;
		}
		int sz = size();
		if (0 == sz) {
			return 0;
		} else if (nLen2move >= sz || nLen2move < 0) {
			nLen2move = sz;
			if (m_cap == pDstRF->m_cap && pDstRF->size() == 0)	{ // swapping two buffers
				char *pttt = m_pBuf;
				m_pBuf = pDstRF->m_pBuf;
				pDstRF->m_pBuf = pttt;
				pDstRF->m_head = m_head;
				pDstRF->m_tail = m_tail;
				m_head = m_tail = 0;
				return sz;
			}
		}
		int rem = m_cap - m_head;
		if (rem >= nLen2move) {
			pDstRF->Append((const char*)m_pBuf + m_head, nLen2move);
		} else {
			pDstRF->Append((const char*)m_pBuf + m_head, rem);
			pDstRF->Append((const char*)m_pBuf, nLen2move - rem);
		}
		m_head = (m_head + nLen2move + m_cap) & m_mod;
		if (m_head == m_tail) {
			m_head = m_tail = 0;
		}
		return nLen2move;
	}

	void Clean()
	{
		Pop(nullptr, -1);
	}
	void Release()
	{
		if (m_pBuf) {
			delete m_pBuf;
			m_pBuf = nullptr;
		}
	}

private:
	int m_cap;
	int m_mod;
	int m_head;
	int m_tail;
	char *m_pBuf;

	static const int s_MaxCapacity = (1 << 24); // 16MB

	static int getFirstPow2LE(int val)
	{
		if (val <= 2) {
			return 2;
		}
		int ret = val;
		while (1) {
			int tt = ret & (ret - 1);
			if (tt != 0) {
				ret = tt;
			} else {
				break;
			}
		}
		if (ret < val) {
			return (ret << 1);
		} else {
			return ret;
		}
	}

	bool adjust(int newCap)
	{
		if (newCap > s_MaxCapacity) {
			return false;
		}
		newCap = getFirstPow2LE(newCap);
		if (newCap == m_cap) {
			return true;
		}
		if (m_pBuf) {
			int sz = size();
			if (sz + 1 > newCap) {
				return false;
			}
			char *pNewBuf = (char*)malloc(newCap);
			if (!pNewBuf) {
				return false;
			}
			if (sz > 0) {
				int rem = m_cap - m_head;
				if (rem >= sz) { // contiguous
					memcpy(pNewBuf, m_pBuf + m_head, sz);
				} else { // around the end
					memcpy(pNewBuf, m_pBuf + m_head, rem);
					memcpy(pNewBuf + rem, m_pBuf, sz - rem);
				}
			}
			delete m_pBuf;
			m_pBuf = pNewBuf;
			m_head = 0;
			m_tail = sz;
		} else {
			m_pBuf = (char*)malloc(newCap);
			if (!m_pBuf) {
				return false;
			}
			m_head = m_tail = 0;
		}
		m_cap = newCap;
		m_mod = m_cap - 1;
		return true;
	}
};

}

#endif // __DDRGENERAL_RING_BUFFER_H_INCLUDED__
