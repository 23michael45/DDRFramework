#include "EventLib.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace DDRMTLib {

struct Event::_impl
{
	bool _bManualReset;
	bool _bSignaled;
	std::mutex _muTex;
	std::condition_variable _condVar;
	int _cntThreadsWaiting;
	_impl(bool bManualReset, bool bSignaled)
	{
		_bManualReset = bManualReset;
		_bSignaled = bSignaled;
		_cntThreadsWaiting = 0;
	}
	void _Set()
	{
		_muTex.lock();
		_bSignaled = true;
		_condVar.notify_all();
		_muTex.unlock();
	}
	void _Reset()
	{
		_muTex.lock();
		_bSignaled = false;
		_muTex.unlock();
	}
	bool _Wait(int nMilliseconds)
	{
		std::unique_lock<std::mutex> lk(_muTex);
		if (_bSignaled) {
			return true;
		}
		++_cntThreadsWaiting;
		bool bSucc = true;
		if (nMilliseconds >= 0) {
			auto _tp = std::chrono::system_clock::now() +
				       std::chrono::milliseconds(nMilliseconds);
			while (1) {
				if (std::cv_status::timeout ==
					_condVar.wait_until(lk, _tp)) {
					bSucc = false;
					break;
				} else if (_bSignaled) {
					bSucc = true;
					break;
				}
			}
		} else {
			while (1) {
				_condVar.wait(lk);
				if (_bSignaled) {
					break;
				}
			}
		}
		if (bSucc) {
			if (--_cntThreadsWaiting == 0 && !_bManualReset) {
				_bSignaled = false;
			}
			return true;
		} else {
			return false;
		}
	}
	bool _WaitUntil(const std::chrono::system_clock::time_point &tp)
	{
		std::unique_lock<std::mutex> lk(_muTex);
		if (_bSignaled) {
			return true;
		}
		++_cntThreadsWaiting;
		bool bSucc = true;
		while (1) {
			if (std::cv_status::timeout ==
				_condVar.wait_until(lk, tp)) {
				bSucc = false;
				break;
			} else if (_bSignaled) {
				bSucc = true;
				break;
			}
		}
		if (bSucc) {
			if (--_cntThreadsWaiting == 0 && !_bManualReset) {
				_bSignaled = false;
			}
			return true;
		} else {
			return false;
		}
	}
};

Event::Event(bool bManualReset, bool bSignaled)
{
	m_pImp = new _impl(bManualReset, bSignaled);
}

Event::~Event()
{
	if (m_pImp) {
		delete m_pImp;
		m_pImp = nullptr;
	}
}

bool Event::Wait(int nWaitTimeInMilliseconds)
{
	if (m_pImp) {
		return m_pImp->_Wait(nWaitTimeInMilliseconds);
	}
	return false;
}

bool Event::WaitUntil(const std::chrono::system_clock::time_point &tp)
{
	if (m_pImp) {
		return m_pImp->_WaitUntil(tp);
	}
	return false;
}

bool Event::Set()
{
	if (m_pImp) {
		m_pImp->_Set();
		return true;
	}
	return false;
}

bool Event::Reset()
{
	if (m_pImp) {
		m_pImp->_Reset();
		return true;
	}
	return false;
}

}