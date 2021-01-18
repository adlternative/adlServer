#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
namespace adl {
class threadPool {
public:
  using Task = std::function<void()>;
  /* 启动size个线程工作 */
  inline threadPool(size_t size = 4);
  /*
    TODO:选择暴力关闭/优雅关闭
  */
  inline ~threadPool();

  // void start(); /* 需要开始么? */
  // void stop();  /* 需要stop么? */

  /* 返回线程数量 */
  size_t size() { return threads.size(); }
  /* 向任务队列添加任务 */
  template <class F, class... Args>
  auto addTask(F &&f, Args &&...args) -> std::future<decltype(f(args...))>;
  /* 返回空闲线程数量 */
  int idlCount() { return idlThrNum.load(); }

private:
  // void take();                      /* 取出任务执行 */
  std::vector<std::thread> threads; /* x个在跑任务的线程？ */
  std::queue<Task> tasks;           /* 任务队列 */
  mutable std::mutex m_mutex;       /* 锁同步 */
  std::condition_variable cv_task;  /* 条件变量 */
  std::atomic<bool>
      running_state; /* 线程池状态 0关闭 1开启后续可以加上立刻关闭*/
  std::atomic<int> idlThrNum; /* 空闲线程数量 */
};
inline threadPool::~threadPool() {
  running_state.store(false);     /* 原子存储!running_state */
  cv_task.notify_all();           /* 通知所有线程死亡的消息 */
  for (auto &&thread : threads) { /* 回收线程资源 */
    if (thread.joinable())
      thread.join();
  }
}

inline threadPool::threadPool(size_t size) : running_state(true) {
  /* 如果size==0 则空闲线程数为1，
  否则就和线程池大小相同 */
  idlThrNum = size < 1 ? 1 : size;
  for (size_t i = 0; i < idlThrNum; i++) {
    threads.emplace_back([this] {
      while (this->running_state) {
        std::function<void()> task;
        {
          /* 加锁 */
          std::unique_lock<std::mutex> ul(this->m_mutex);
          /* 等待任务队列非空或者结束条件 */
          this->cv_task.wait(ul, [this] {
            return !this->running_state.load() || !this->tasks.empty();
          });
          /* 如果现在线程池需要结束而且当前的任务队列是空的则返回
           * 这意味着如果任务不是空的，线程池会执行到全部任务结束
           *为止。
           */
          if (!this->running_state && this->tasks.empty())
            return;
          /* 取出一个任务 */
          task = std::move(this->tasks.front());
          this->tasks.pop();
        }
        idlThrNum--; /* 空闲线程数-- */
        task();      /* 执行任务 */
        idlThrNum++; /* 空闲线程数++*/
      }
    });
  }
}

template <class F, class... Args>
auto threadPool::addTask(F &&f, Args &&...args)
    -> std::future<decltype(f(args...))> {
  if (!running_state.load()) // 线程池 已经停止
    throw std::runtime_error("commit on threadPool is stopped.");
  using RetType = decltype(f(args...));
  /* 这个地方用了用智能指针指向 packaged_task 之后返回future*/
  auto task = std::make_shared<std::packaged_task<RetType()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  std::future<RetType> future = task->get_future(); /* 为调用者提供返回值 */
  {
    /*在锁中向任务队列添加任务  */
    std::lock_guard<std::mutex> lock{m_mutex};
    tasks.emplace([task]() { (*task)(); });
  }
  cv_task.notify_one(); /* 通知一个等待的线程起来工作 */
  return future;
}

} // namespace adl
#endif
