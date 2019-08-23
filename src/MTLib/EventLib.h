#ifndef __DDR_EVENT_LIB_CPP11_H_INCLUDED__
#define __DDR_EVENT_LIB_CPP11_H_INCLUDED__

#include <chrono>

namespace DDRMTLib {

// *********** C++ 11 wrapper (cross-platform) for windows-style event ***********
class Event
{
public:
	Event(bool bManualReset = true, bool bSignaled = false);
	~Event();
	bool Wait(int nWaitTimeInMilliseconds);
	bool WaitUntil(const std::chrono::system_clock::time_point &tp);
	bool Set();
	bool Reset();
private:
	struct _impl;
	_impl *m_pImp;
};


}

#endif // __DDR_EVENT_LIB_CPP11_H_INCLUDED__