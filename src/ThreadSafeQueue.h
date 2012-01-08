#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <deque>
#include <boost/thread/mutex.hpp>

template<class T> class ThreadSafeQueue
{
	
private:
	
    std::deque<T> m_queue;
    boost::mutex m_mutex;
	int m_max;
	
public:
	
	ThreadSafeQueue(uint m = -1)
	: m_max(m)
	{
	}
	
	virtual ~ThreadSafeQueue()
	{
		clear();
	}
	
	inline uint count() 
	{
		boost::mutex::scoped_lock lock(m_mutex);
		int count = m_queue.size();
		return count;
	}
	
	inline bool isFull() 
	{
		if (-1 == m_max)
			return false;
		
        boost::mutex::scoped_lock lock(m_mutex);
		int count = m_queue.size();
		return count >= m_max;
	}
	
	inline bool isEmpty() 
	{
		boost::mutex::scoped_lock lock(m_mutex);
		bool empty = m_queue.isEmpty();
		return empty;
	}
	
	inline void clear()
	{
        boost::mutex::scoped_lock lock(m_mutex);
		m_queue.clear();
	}
	
	inline void push(const T& t)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		m_queue.push_back(t);
	}
	
	inline T pop()
	{
		boost::mutex::scoped_lock lock(m_mutex);
		T i = m_queue.front();
        m_queue.pop_front();
        
		return i;
	}
	

};
#endif //THREADSAFEQUEUE_H