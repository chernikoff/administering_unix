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

  T& take()
  {
    std::unique_lock < std::mutex > lock ( m_mutex );
    return m_queue.front();
  }

  void pop()
  {
    std::unique_lock < std::mutex > lock ( m_mutex );
    m_queue.pop();
  }

  void push( const T &item )
  {
    std::unique_lock < std::mutex > lock( m_mutex );
    m_queue.push( item );
    lock.unlock();
    m_cond.notify_all();
  }

  bool isOpen() const
  {
    return m_is_open;
  }

  void close()
  {
    m_is_open = false;
    m_cond.notify_all();
  }

  bool hasNext()
  {
    std::unique_lock < std::mutex > lock( m_mutex );
    return !m_queue.empty();
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
