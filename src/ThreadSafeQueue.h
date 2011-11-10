#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <QThread>
#include <QMutex>
#include <QQueue>

template<class T> class ThreadSafeQueue
{
	
private:
	
	QQueue<T> m_queue;
	QMutex m_mutex;
	int m_max;
	
public:
	
	ThreadSafeQueue(uint m = -1)
	: m_max(m)
	{
	}
	
	~ThreadSafeQueue()
	{
		clear();
	}
	
	inline uint count() 
	{
		m_mutex.lock();
		int count = m_queue.count();
		m_mutex.unlock();
		return count;
	}
	
	inline bool isFull() 
	{
		if (-1 == m_max)
			return false;
		
		m_mutex.lock();
		int count = m_queue.count();
		m_mutex.unlock();
		return count >= m_max;
	}
	
	inline bool isEmpty() 
	{
		m_mutex.lock();
		bool empty = m_queue.isEmpty();
		m_mutex.unlock();
		return empty;
	}
	
	inline void clear()
	{
		m_mutex.lock();
		m_queue.clear();
		m_mutex.unlock();
	}
	
	inline void push(const T& t)
	{
		m_mutex.lock();
		m_queue.enqueue(t);
		m_mutex.unlock();
	}
	
	inline T pop()
	{
		m_mutex.lock();
		T i = m_queue.dequeue();
		m_mutex.unlock();
		return i;
	}
	

};
#endif //THREADSAFEQUEUE_H