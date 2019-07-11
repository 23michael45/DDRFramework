#ifndef __DDRVISION_ADAPTIVE_DOUBLE_QUEUE_H_INCLUDED__
#define __DDRVISION_ADAPTIVE_DOUBLE_QUEUE_H_INCLUDED__

namespace DDRCloudService {

template <typename T> class AdaptiveDequeue {
public:
	AdaptiveDequeue(int cap = 16) : m_head(0), m_tail(0), m_it(0), m_cap(cap <= 1 ? 1 : cap) {
		m_data.resize(m_cap + 1);
	}
	void clear(bool bReleaseMem = false) {
		if (bReleaseMem) {
			m_data.clear();
		} else {
			m_data.resize(0);
		}
		m_head = m_tail = m_cap = 0;
	}
	bool empty() const {
		return (m_tail == m_head);
	}
	size_t size() const {
		return (size_t)(m_tail + m_cap + 1 - m_head) % (m_cap + 1);
	}
	void emplace_back(const T &t) {
		_expand();
		m_data[m_tail] = t;
		m_tail = (m_tail + 1) % (m_cap + 1);
	}
	void emplace_back(T &&t) {
		_expand();
		m_data[m_tail] = t;
		m_tail = (m_tail + 1) % (m_cap + 1);
	}
	bool pop_back(T &t) {
		if (m_head != m_tail) {
			m_tail = (m_tail + m_cap) % (m_cap + 1);
			t = std::move(m_data[m_tail]);
			if (m_head == m_tail) {
				m_head = m_tail = 0;
			}
			return true;
		}
		return false;
	}
	void emplace_front(const T &t) {
		_expand();
		m_head = (m_head + m_cap) % (m_cap + 1);
		m_data[m_head] = t;
	}
	void emplace_front(T &&t) {
		_expand();
		m_head = (m_head + m_cap) % (m_cap + 1);
		m_data[m_head] = t;
	}
	bool pop_front(T &t) {
		if (m_head != m_tail) {
			t = std::move(m_data[m_head]);
			m_head = (m_head + 1) % (m_cap + 1);
			if (m_head == m_tail) {
				m_head = m_tail = 0;
			}
			return true;
		}
		return false;
	}
	void ResetIterator() {
		m_it = 0;
	}
	const T* GetNextIterator() {
		if (m_it < size()) {
			return &m_data[(m_head + (m_it++)) % (m_cap + 1)];
		}
		return nullptr;
	}
protected:
	std::vector<T> m_data;
	int m_cap, m_head, m_tail;
	int m_it;
	void _expand() {
		if ((m_tail + m_cap + 1 - m_head) % (m_cap + 1) == m_cap) {
			int newCap = (m_cap + 1) << 1;
			m_data.resize(newCap);
			if (m_tail < m_head) { // data across old boundary
				for (int i = 0; i < m_tail; ++i) {
					m_data[m_cap + 1 + i] = std::move(m_data[i]);
				}
				m_tail = m_head + m_cap;
			}
			m_cap = newCap - 1;
		}
	}
};

}

#endif // __DDRVISION_ADAPTIVE_DOUBLE_QUEUE_H_INCLUDED__