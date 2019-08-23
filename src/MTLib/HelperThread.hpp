#ifndef __DDR_HELPER_THREAD_EASY_TASK_ASSIGNMENT_H_INCLUDED__
#define __DDR_HELPER_THREAD_EASY_TASK_ASSIGNMENT_H_INCLUDED__

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace DDRMTLib {

class HelperThread
{
public:
	typedef void(*_MY_FUNC_)(void*);
	HelperThread() : m_bQuit(false), m_bToQuit(false), m_tHead(0), m_tTail(0)
	{
		m_thr = std::thread([this]() {
			while (!m_bQuit) {
				std::pair<_MY_FUNC_, void*> nextTask;
				{
					std::unique_lock<std::mutex> lg(m_qLoc);
					m_newTaskNotif.wait(lg, [this] { return (m_bQuit || !m_tasks.empty());});
					if (m_bQuit) {
						return;
					}
					nextTask = std::move(m_tasks.front());
					m_tasks.pop();
					if (!m_tasks.empty()) {
						m_newTaskNotif.notify_one();
					}
				}
				nextTask.first(nextTask.second);
				{
					std::lock_guard<std::mutex> lg(m_qLoc);
					++m_tHead;
				}
				m_taskFinNotif.notify_all();
			}
		});
	}
	~HelperThread()
	{
		Quit(false);
	}

	bool AssignTask(_MY_FUNC_ funcPtr, void *pArg, unsigned int *pTaskID = nullptr)
	{
		if (m_bToQuit) {
			return false;
		}
		std::lock_guard<std::mutex> lg2(m_qLoc);
		if (pTaskID) {
			*pTaskID = m_tTail;
		}
		++m_tTail;
		m_tasks.emplace(std::pair<_MY_FUNC_, void*>(funcPtr, pArg));
		m_newTaskNotif.notify_one();
		return true;
	}

	bool HasFinishedTask(unsigned int taskID)
	{
		std::lock_guard<std::mutex> lg(m_qLoc);
		return ((m_tTail == m_tHead) ||
			    (m_tTail > m_tHead && (taskID < m_tHead || taskID >= m_tTail)) ||
			    (m_tTail < m_tHead && (taskID < m_tHead && taskID >= m_tTail)));
	}

	bool HasFinishedAllTasks()
	{
		std::lock_guard<std::mutex> lg(m_qLoc);
		return (m_tTail == m_tHead);
	}

	void WaitForTask(unsigned int taskID)
	{
		std::unique_lock<std::mutex> ul(m_qLoc);
		m_taskFinNotif.wait(ul, [this, taskID] { return ((m_tTail == m_tHead) ||
			                                             (m_tTail > m_tHead && (taskID < m_tHead || taskID >= m_tTail)) ||
			                                             (m_tTail < m_tHead && (taskID < m_tHead && taskID >= m_tTail))); });
	}

	void WaitForAllTasks()
	{
		std::unique_lock<std::mutex> ul(m_qLoc);
		m_taskFinNotif.wait(ul, [this] { return (m_tTail == m_tHead); });
	}

	void Quit(bool bWait4InQueueTasks = true)
	{
		bool bFALSE = false;
		if (!m_bToQuit.compare_exchange_strong(bFALSE, true)) {
			return;
		}
		if (bWait4InQueueTasks) {
			WaitForAllTasks();
		}
		m_bQuit = true;
		m_newTaskNotif.notify_one();
		if (m_thr.joinable()) {
			m_thr.join();
		}
	}

private:
	bool m_bQuit;
	std::atomic<bool> m_bToQuit;
	std::thread m_thr;

	std::mutex m_qLoc;
	std::queue<std::pair<_MY_FUNC_, void*>> m_tasks;
	std::condition_variable m_newTaskNotif;
	std::condition_variable m_taskFinNotif;
	unsigned int m_tHead;
	unsigned int m_tTail;
};

}

#endif // __DDR_HELPER_THREAD_EASY_TASK_ASSIGNMENT_H_INCLUDED__

