#include "EventLoop.h"
#include "../include/headFile.h"
#include "Channel.h"
#include "Socket.h"
#include <sys/eventfd.h>
/* SIGPIPE */

/* 创建事件描述符 */

const int kPollTimeMs = 10000; /* 10s */

int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG(FATAL) << "createEventfd error" << adl::endl;
  }
  return evtfd;
}

using namespace adl;

EventLoop::EventLoop(bool mainLoop)
    : isMainLoop_(mainLoop), threadId_(CurrentThread::tid()),
      currentActiveChannel_(nullptr), quit_(false), eventHandling_(false),
      callingPendingFunctors_(false) {}

EventLoop::~EventLoop() {
  wakeupChannel_->disableAll(); /* 从Epoll中删除wakeupChannel_ */
  wakeupChannel_->remove();     /* 彻底在map中删除wakeupChannel_ */
  ::close(wakeupFd_);
}

/* 在构造函数中将this 生成shared_from_this()会出错，
  二段构造 */
void EventLoop::init() {
  INFO_("EventLoop.init\n");
  /* poller 必须先于 channel 初始化 ，
    否则会出错 */
  poller_ = std::make_unique<Epoller>(shared_from_this());
  wakeupFd_ = createEventfd();
  wakeupChannel_ = std::make_unique<Channel>(shared_from_this(), wakeupFd_);
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

void EventLoop::quit() {
  quit_ = true;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  /* EventLoop在析构函数之后后某个线程调用了quit()
  可能会出现错误 */
  if (!isInLoopThread()) {
    wakeup();
  }
}

/* 加锁后向队列中添加任务函数
如果不是在IO线程或者说现在是在IO线程，但我们正在执行任务
（我们的新任务并不在当前交换的局部任务向量中），
我们需要额外的一次唤醒操作（下次唤醒）
那么唤醒epoll_wait */
void EventLoop::queueInLoop(Functor cb) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.push_back(std::move(cb));
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = sock::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG(ERROR) << "EventLoop::wakeup error" << adl::endl;
  }
}

void EventLoop::updateChannel(Channel *channel) {
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  assertInLoopThread();
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::addConnect(std::shared_ptr<TcpConnection> con) {
  runInLoop([this, con]() { addConnectInLoop(con); });
}

void EventLoop::rmConnect(std::shared_ptr<TcpConnection> con) {
  runInLoop([this, con]() { rmConnectInLoop(con); });
}

void EventLoop::addConnectInLoop(std::shared_ptr<TcpConnection> con) {
  assertInLoopThread();
  connectSet_.insert(con);
}

void EventLoop::rmConnectInLoop(std::shared_ptr<TcpConnection> con) {
  assertInLoopThread();
  connectSet_.erase(con);
}

void EventLoop::assertInLoopThread() {
  if (!isInLoopThread()) {
    LOG(FATAL) << "not in specified EventLoop"
               << (isMainLoop() ? "(mainLoop)" : "(subLoop)") << "\n";
    // abortNotInLoopThread();
  }
}

/* 如果在IO线程，则直接执行任务函数，
否则添加供IO线程执行的任务队列中
（其他线程需要让IO线程完成的异步任务）*/
void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::loop() {
  quit_ = false;
  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    eventHandling_ = true;
    /* 处理所有活跃通道的事件 */
    for (Channel *channel : activeChannels_) {
      currentActiveChannel_ = channel;
      currentActiveChannel_->handleEvent();
    }
    currentActiveChannel_ = NULL;
    eventHandling_ = false;

    doPendingFunctors(); /* 小任务列表 */
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sock::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG(ERROR) << "EventLoop::handleRead(wakeup) error" << adl::endl;
  }
}

void EventLoop::doPendingFunctors() {

  std::vector<Functor> functors;
  callingPendingFunctors_ = true;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (const Functor &functor : functors) {
    functor();
  }
  callingPendingFunctors_ = false;
}