#ifndef __DDR_HELPER_THREAD_STANDBY_THREAD_H_INCLUDED__
#define __DDR_HELPER_THREAD_STANDBY_THREAD_H_INCLUDED__

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace DDRMTLib {

// Derive from this class and define _process() as the core processing function in the subthread
// Call startProcessing() if all things are prepared for the subthread to process
// WARNING: startProcessing() is just a notification, so the caller should NOT hold any MUTEX
//          before calling it; otherwise, dead locking can happen!
class StandByThread
{
public:
	StandByThread(bool bDetachable = false) :
		m_bDetachable(bDetachable), m_bQuit(false), m_bNewTask(false), m_bBusy(false)
	{
		startSubThread();
	}
	virtual ~StandByThread() { Stop(); };
	void Restart()
	{
		Stop();
		m_bQuit = false;
		startSubThread();
	}
	void Stop()
	{
		if (!m_bQuit) {
			m_bQuit = true;
			if (m_thr.joinable()) {
				if (!m_bDetachable) {
					m_newTaskNotif.notify_one();
					m_thr.join();
				} else {
					m_thr.detach();
				}
			}
			m_bBusy = false;
		}
	}
	bool IsDetachable() const { return m_bDetachable; }
	bool IsBusy() const { return m_bBusy; }

protected:
	virtual void _process() = 0;
	void startProcessing()
	{
		{
			std::lock_guard<std::mutex> lg2(m_loc);
			m_bNewTask = true;
		}
		m_newTaskNotif.notify_one();
	}

private:
	bool m_bDetachable;
	std::atomic<bool> m_bQuit;
	bool m_bNewTask;
	std::atomic<bool> m_bBusy;
	std::mutex m_loc;
	std::condition_variable m_newTaskNotif;
	std::thread m_thr;

	void startSubThread()
	{
		m_thr = std::thread([this]() {
			while (!m_bQuit) {
				{
					{
						std::unique_lock<std::mutex> lg(m_loc);
						m_newTaskNotif.wait(lg, [this] {
							if (!m_bQuit) {
								if (m_bNewTask) {
									m_bNewTask = false;
									m_bBusy = true;
									return true;
								}
								return false;
							}
							return true;
						});
					}
					if (m_bQuit) {
						break;
					} else {
						_process();
						m_bBusy = false;
					}
				}
			}
		});
	}
};

}

#endif // __DDR_HELPER_THREAD_STANDBY_THREAD_H_INCLUDED__

