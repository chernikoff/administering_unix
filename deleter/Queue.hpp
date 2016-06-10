#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

template < typename T >
class Queue {
public:
  Queue()
    : m_is_open( true )
    {}

  bool getNext(T& next)
  {
    std::unique_lock < std::mutex > lock ( m_mutex );
    while (m_is_open) {
	if (m_queue.empty()) {
            lock.unlock();
	    sleep(1);
            lock.try_lock();
   	} else {
	    break;
	}
    }
    if (m_queue.empty()) {
	    return false;
    }
    next = m_queue.front();
    m_queue.pop();
    return true;
  }

  void push( const T &item )
  {
    std::unique_lock < std::mutex > lock( m_mutex );
    m_queue.push( item );
    lock.unlock();
    m_cond.notify_all();
  }

  void close()
  {
    m_is_open = false;
    m_cond.notify_all();
  }

  void wait()
  {
    std::unique_lock < std::mutex > lock( m_mutex );
    m_cond.wait( lock );
  }

private:
  std::queue < T >         m_queue;
  std::mutex               m_mutex;
  std::condition_variable  m_cond;
  std::atomic < bool >     m_is_open;
};
