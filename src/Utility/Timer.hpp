#ifndef Timer_h__
#define Timer_h__




#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <algorithm>
#include <vector>
#include <stack>
#include <set>

namespace DDRFramework
{

	// Public types
	using timer_id = std::size_t;
	using handler_t = std::function<void(timer_id)>;
	using clock = std::chrono::steady_clock;
	using timestamp = std::chrono::time_point<clock>;
	using duration = std::chrono::microseconds;

	// Private definitions. Do not rely on this namespace.
	namespace detail
	{

		// The event structure that holds the information about a timer.
		struct Event {
			timer_id id;
			timestamp start;
			duration period;
			handler_t handler;
			bool valid;
			Event()
				: id(0), start(duration::zero()), period(duration::zero()), handler(nullptr), valid(false)
			{
			}
			template <typename Func>
			Event(timer_id id, timestamp start, duration period, Func &&handler)
				: id(id), start(start), period(period), handler(std::forward<Func>(handler)), valid(true)
			{
			}
			Event(Event &&r) = default;
			Event &operator=(Event &&ev) = default;
			Event(const Event &r) = delete;
			Event &operator=(const Event &r) = delete;
		};

		// A time event structure that holds the next timeout and a reference to its
		// Event struct.
		struct Time_event {
			timestamp next;
			timer_id ref;
			duration interval;
		};

		inline bool operator<(const Time_event &l, const Time_event &r)
		{
			return l.next < r.next;
		}

	} // end namespace detail

	class Timer
	{
		using scoped_m = std::unique_lock<std::mutex>;

		// Thread and locking variables.
		std::mutex m;
		std::condition_variable cond;
		std::thread worker;

		// Use to terminate the timer thread.
		bool done = false;

		// The vector that holds all active events.
		std::vector<detail::Event> events;
		// Sorted queue that has the next timeout at its top.
		std::multiset<detail::Time_event> time_events;

		// A list of ids to be re-used. If possible, ids are used from this pool.
		std::stack<timer_id> free_ids;

	public:
		Timer() : m{}, cond{}, worker{}, events{}, time_events{}, free_ids{}
		{
			scoped_m lock(m);
			done = false;
			worker = std::thread(std::bind(&Timer::run, this));
		}

		~Timer()
		{
			scoped_m lock(m);
			done = true;
			lock.unlock();
			cond.notify_all();
			worker.join();
			events.clear();
			time_events.clear();
			while (!free_ids.empty()) {
				free_ids.pop();
			}
		}

		/**
		 * Add a new timer.
		 *
		 * \param when The time at which the handler is invoked.
		 * \param handler The callable that is invoked when the timer fires.
		 * \param period The periodicity at which the timer fires. Only used for periodic timers.
		 */
		timer_id add(
			const timestamp &when, handler_t &&handler,  const duration d, const duration &period = duration::zero())
		{

			scoped_m lock(m);
			timer_id id = 0;
			// Add a new event. Prefer an existing and free id. If none is available, add
			// a new one.
			if (free_ids.empty()) {
				id = events.size();
				detail::Event e(id, when, period, std::move(handler));
				events.push_back(std::move(e));
			}
			else {
				id = free_ids.top();
				free_ids.pop();
				detail::Event e(id, when, period, std::move(handler));
				events[id] = std::move(e);
			}
			time_events.insert(detail::Time_event{ when, id ,d });
			lock.unlock();
			cond.notify_all();
			return id;
		}

		/**
		 * Overloaded `add` function that uses a `std::chrono::duration` instead of a
		 * `time_point` for the first timeout.
		 */
		template <class Rep, class Period>
		inline timer_id add(const std::chrono::duration<Rep, Period> &when, handler_t &&handler,
			const duration &period = duration::zero())
		{
			return add(clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(when),
				std::move(handler),  std::chrono::duration_cast<std::chrono::microseconds>(when), period);
		}

		/**
		 * Overloaded `add` function that uses a uint64_t instead of a `time_point` for
		 * the first timeout and the period.
		 */
		inline timer_id add(const uint64_t when, handler_t &&handler, const uint64_t period = 0)
		{
			return add(duration(when), std::move(handler), duration(period));
		}

		/**
		 * Removes the timer with the given id.
		 */
		bool remove(timer_id id)
		{
			scoped_m lock(m);
			if (events.size() == 0 || events.size() < id) {
				return false;
			}
			events[id].valid = false;
			auto it = std::find_if(time_events.begin(), time_events.end(),
				[&](const detail::Time_event &te) { return te.ref == id; });
			if (it != time_events.end()) {
				free_ids.push(it->ref);
				time_events.erase(it);
			}
			lock.unlock();
			cond.notify_all();
			return true;
		}
		void reset(timer_id id)
		{
			scoped_m lock(m);
			if (events.size() == 0 || events.size() < id) {
				return ;
			}
			events[id].valid = false;

			auto it = std::find_if(time_events.begin(), time_events.end(),
				[&](const detail::Time_event &te) { return te.ref == id; });

			if (it != time_events.end()) {
				
			    timestamp when = clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(it->interval);
				auto newevent = detail::Time_event{ when, id ,it->interval };

				time_events.erase(*it);

				time_events.insert(newevent);

			}
			lock.unlock();
			cond.notify_all();
			return;

		}

	private:
		void run()
		{
			scoped_m lock(m);

			while (!done) {

				if (time_events.empty()) {
					// Wait for work
					cond.wait(lock);
				}
				else {
					detail::Time_event te = *time_events.begin();
					if (clock::now() >= te.next) {

						// Remove time event
						time_events.erase(time_events.begin());

						// Invoke the handler
						lock.unlock();
						events[te.ref].handler(te.ref);
						lock.lock();

						if (events[te.ref].valid && events[te.ref].period.count() > 0) {
							// The event is valid and a periodic timer.
							te.next += events[te.ref].period;
							time_events.insert(te);
						}
						else {
							// The event is either no longer valid because it was removed in the
							// callback, or it is a one-shot timer.
							events[te.ref].valid = false;
							free_ids.push(te.ref);
						}
					}
					else {
						cond.wait_until(lock, te.next);
					}
				}
			}
		}
	};

}

#endif // Timer_h__