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

  T pop()
  {
    std::unique_lock < std::mutex > lock( m_mutex );
    while( m_is_open && m_queue.empty() ) {
      m_cond.wait( lock );
    }
    auto item = m_queue.front();
    m_queue.pop();
    return item;
  }

  void push( const T &item )
  {
    std::unique_lock < std::mutex > lock( m_mutex );
    m_queue.push( item );
    lock.unlock();
    m_cond.notify_one();
  }

  bool isOpen() const
  {
    return m_is_open || !m_queue.empty();
  }

  void close()
  {
    m_is_open = false;
    m_cond.notify_one();
  }

private:
  std::queue < T >         m_queue;
  std::mutex               m_mutex;
  std::condition_variable  m_cond;
  std::atomic < bool >     m_is_open;
};
