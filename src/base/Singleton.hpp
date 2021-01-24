#include <atomic>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <pthread.h>

namespace adl {

namespace learn_from_muduo {
/* 多个线程只会初始化一次Singleton<T>，
 *就是只会执行一次init,之后直接通过这个类的instance
 *获得全局唯一对象（指定的T类型）
 *可以有多个不同类型的单例全局对象同时存在
 */
template <typename T> class Singleton : boost::noncopyable {
public:
  static T &instance() {
    pthread_once(&ponce_once, &Singleton::init);
    return *value_;
  }

private:
  Singleton();
  ~Singleton();
  static void init() { value_ = new T(); }
  static pthread_once_t ponce_once;
  static T *value_;
};

template <typename T>
pthread_once_t Singleton<T>::ponce_once =
    PTHREAD_ONCE_INIT; /* 在头文件中就指定了这个类的static变量的初始化 */
template <typename T> T *Singleton<T>::value_ = NULL; /* value就指向一个NULL */
} // namespace learn_from_muduo

namespace use_memory_barrier {
class Singleton : boost::noncopyable {
public:
  static Singleton *getInstance();
  static std::atomic<Singleton *> m_instance;
  static std::mutex m_mutex;
};
std::atomic<Singleton *> Singleton::m_instance;
std::mutex Singleton::m_mutex;

Singleton *Singleton::getInstance() {
  Singleton *tmp = m_instance.load(std::memory_order_relaxed); /* 原子,自由 读*/
  std::atomic_thread_fence(
      std::memory_order_acquire); /* 原子,屏障,acq(后不往前) */
  if (tmp == nullptr) {
    std::lock_guard<std::mutex> lock(m_mutex);
    tmp = m_instance.load(std::memory_order_relaxed); /* 原子,自由,读 */
    if (tmp == nullptr) {
      tmp = new Singleton;
      std::atomic_thread_fence(
          std::memory_order_release); /* 原子,屏障,rel(前不往后) */
      /* 注意到这个前不向后的用途就是tmp如果生成指针重排到构造内容之前，
       *这些写的步骤不会重拍到
       *atomic_thread_fence之后。
       *因此:m_instance会获得到
       *正确构造的数据。
       */
      m_instance.store(tmp, std::memory_order_relaxed); /* 原子,自由,写 */
    }
  }
  return tmp;
}
} // namespace use_memory_barrier

namespace learn_from_starkoverflow {
template <typename T> struct lazy {
public:
  template <typename Fun>
  explicit lazy(Fun &&fun) : fun(std::forward<Fun>(fun)) {}

  T &get() {
    std::call_once(flag, [this] { ptr.reset(fun()); });
    return *ptr;
  }
  // more stuff like op* and op->, implemented in terms of get()

private:
  mutable std::once_flag flag;
  mutable std::unique_ptr<T> ptr;
  std::function<T *()> fun;
};
} // namespace learn_from_starkoverflow
namespace learn_from_csdn {
template <typename T> class Singleton {
private:
  static std::atomic<T *> m_instance;
  static std::mutex m_mutex;

public:
  static T *getInstance();
};

template <typename T> std::atomic<T *> Singleton<T>::m_instance;
template <typename T> std::mutex Singleton<T>::m_mutex;

template <typename T> T *Singleton<T>::getInstance() {
  T *tmp = m_instance.load(std::memory_order_acquire);
  // atomic_thread_fence (std::memory_order_acquire);
  if (tmp == nullptr) {

    std::lock_guard<std::mutex> lock(m_mutex);
    if (tmp == nullptr) {
      tmp = new T;
      // atomic_thread_fence  (std::memory_order_release);
      m_instance.store(tmp, std::memory_order_release);
    }
  }
  return tmp;
}
} // namespace learn_from_csdn
} // namespace adl
