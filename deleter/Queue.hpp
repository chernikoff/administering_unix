#include <queue>
#include <mutex>
#include <condition_variable>

template < typename T >
class Queue {
public:
  T pop()
  {
    std::unique_lock < std::mutex > lock( m_mutex );
    while( m_queue.empty() ) {
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

private:
  std::queue < T >         m_queue;
  std::mutex               m_mutex;
  std:: condition_variable m_cond;
};
