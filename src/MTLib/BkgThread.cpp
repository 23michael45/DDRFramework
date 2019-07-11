#include "BkgThread.h"

#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

//#include <iostream>

namespace DDRMTLib {

class BkgThread
{
public:
	BkgThread() = delete;
	BkgThread(void(*pFunc)(void*, bool*), void *pArg, std::condition_variable *pCV)
		: m_bQuit(false), m_bFin(false), m_pFunc(pFunc), m_pArg(pArg), m_pCV(pCV)
	{
		if (pCV) {
			m_th = std::thread(thFunc, this);
		}
	}
	BkgThread(const BkgThread&) = delete;
	BkgThread(BkgThread &&ori) = delete;
	~BkgThread()
	{
		if (!Try2Join()) {
			m_th.detach();
		}
	}

	bool Try2Join()
	{
		if (m_th.joinable()) {
			if (!m_bFin) {
				return false;
			} else {
				m_th.join();
			}
		}
		return true;
	}

	void Ask2Quit()
	{
		m_bQuit = true;
	}

private:
	std::thread m_th;
	void(*m_pFunc)(void*, bool*);
	void *m_pArg;

	bool m_bQuit;
	std::atomic<bool> m_bFin;
	std::condition_variable *m_pCV;
	void _thFunc()
	{
		m_pFunc(m_pArg, &m_bQuit);
		m_bFin = true;
		m_pCV->notify_one();
	}
	static void thFunc(void *pArg)
	{
		((BkgThread*)pArg)->_thFunc();
	}
};

class BkgThreadMgt
{
public:
	BkgThreadMgt() : m_curThID(0), m_bQuit(false)
	{
		m_mgtTh = std::thread(&BkgThreadMgt::_thFunc, this);
	}
	~BkgThreadMgt()
	{
		if (m_mgtTh.joinable()) {
			m_bQuit = true;
			m_cv.notify_one();
			m_mgtTh.join();
		}
	}

	unsigned int AddThread(void(*pFunc)(void*, bool*), void *pArg)
	{
		std::lock_guard<std::mutex> lg(m_loc);
		m_IDMapper[m_curThID] = (int)m_threads.size();
		auto pNewBkgTh = std::make_shared<BkgThread>(pFunc, pArg, &m_cv);
		m_threads.emplace_back(OneRegisteredBkgThread({ pNewBkgTh, m_curThID }));
		unsigned int ret = m_curThID;
		if (++m_curThID == -1) {
			++m_curThID;
		}
		return ret;
	}

	// true - thread with thID not running; false - timed-out
	bool Wait4Thread(unsigned int thID, int waitTimeMilsec)
	{
		std::unique_lock<std::mutex> ul(m_loc);
		if (m_IDMapper.find(thID) == m_IDMapper.end()) {
			return true;
		}
		std::condition_variable mycv;
		m_waitingList.emplace_back(&mycv);
		m_cv.notify_one();
		if (waitTimeMilsec < 0) {
			while (true) {
				mycv.wait(ul);
				if (m_IDMapper.find(thID) == m_IDMapper.end()) {
					_removeCVFromList(&mycv);
					return true;
				}
				m_cv.notify_one();
			}
		} else {
			auto tic1 = std::chrono::system_clock::now() + std::chrono::milliseconds(waitTimeMilsec);
			while (true) {
				if (std::cv_status::timeout == mycv.wait_until(ul, tic1)) {
					_removeCVFromList(&mycv);
					return false;
				}
				if (m_IDMapper.find(thID) == m_IDMapper.end()) {
					_removeCVFromList(&mycv);
					return true;
				}
				m_cv.notify_one();
			}
		}
	}

private:
	struct OneRegisteredBkgThread
	{
		std::shared_ptr<BkgThread> pTh;
		unsigned int thID;
	};
	std::vector<OneRegisteredBkgThread> m_threads;
	std::map<unsigned int, int> m_IDMapper;
	unsigned int m_curThID;
	std::mutex m_loc;
	std::condition_variable m_cv;
	std::vector<std::condition_variable*> m_waitingList;

	std::atomic<bool> m_bQuit;
	std::thread m_mgtTh;
	void _thFunc()
	{
		std::unique_lock<std::mutex> ul(m_loc);
		while (1) {
			m_cv.wait(ul);
			if (!m_bQuit) {
				bool bSomeThEnding = false;
				for (int i = (int)m_threads.size() - 1; i >= 0; --i) {
					if (m_threads[i].pTh->Try2Join()) {
						auto ind = m_IDMapper.find(m_threads[i].thID);
						if (ind != m_IDMapper.end()) {
							m_IDMapper.erase(ind);
						}
						m_threads.erase(m_threads.begin() + i);
						bSomeThEnding = true;
					}
				}
				if (bSomeThEnding) {
					for (auto x : m_waitingList) {
						x->notify_one();
					}
				}
			} else {
				for (int i = (int)m_threads.size() - 1; i >= 0; --i) {
					m_threads[i].pTh->Ask2Quit();
				}
				for (int i = (int)m_threads.size() - 1; i >= 0; --i) {
					if (m_threads[i].pTh->Try2Join()) {
						m_threads.erase(m_threads.begin() + i);
					}
				}
				if (!m_threads.empty()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					m_threads.clear();
				}
				break;
			}
		}
	}

	void _removeCVFromList(std::condition_variable *pCV)
	{
		for (int i = 0; i < (int)m_waitingList.size(); ++i) {
			if (m_waitingList[i] == pCV) {
				m_waitingList.erase(m_waitingList.begin() + i);
			}
		}
	}
};

BkgThreadMgt *g_pBkgThMgt = nullptr;
void __cleaner()
{
	if (g_pBkgThMgt) {
		delete g_pBkgThMgt;
	}
}

class _StarterHelper {
public:
	_StarterHelper() {
		g_pBkgThMgt = new BkgThreadMgt();
		atexit(__cleaner);
	}
} __starter_helper__;


bool CreateBkgThread(void(*pFunc)(void*, bool*), void *pArg, unsigned int *pThreadID)
{
	if (g_pBkgThMgt) {
		unsigned int thID = g_pBkgThMgt->AddThread(pFunc, pArg);
		if (pThreadID) {
			*pThreadID = thID;
		}
		return true;
	}
	return false;
}

bool Wait4BkgThread(unsigned int bkgThID, int waitTimeMilisec)
{
	if (g_pBkgThMgt) {
		return g_pBkgThMgt->Wait4Thread(bkgThID, waitTimeMilisec);
	}
	return true;
}

}