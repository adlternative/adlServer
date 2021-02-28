#include "Epoller.h"
#include "../headFile.h"
#include "Channel.h"

using namespace adl;

Epoller::Epoller(EventLoop *loop)
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(16), ownerLoop_(loop) {
  if (epollfd_ == -1) {
    LOG(FATAL) << "epoll_create1 error" << adl::endl;
  }
}

Epoller::~Epoller() { ::close(epollfd_); }

/* epoll_wait */
timeStamp Epoller::poll(int timeoutMs, ChannelList *activeChannels) {
  /* Log */
  LOG(INFO) << "epoll begin to wait" << adl::endl;
  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                               static_cast<int>(events_.size()), timeoutMs);
  int savedErrno = errno; /* 保存当前错误，以防被之后的函数修改 */
  timeStamp now(timeStamp::now());
  LOG(INFO) << "epoll wait over" << adl::endl;
  if (numEvents < 0) {
    errno = savedErrno;
    LOG(ERROR) << "Epoller::poll error" << adl::endl;
  } else if (numEvents == 0) {
    LOG(TRACE) << "Epoller::poll timeout, no events happened!" << adl::endl;
    /* 如果我们设置了超时时间，
    并且在这段时间内没有事件发生 */
  } else {
    fillActiveChannels(numEvents, activeChannels); /* 填充活跃的通道 */
    /* 扩容？ */
    if (implicit_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  }
  return now;
}

void Epoller::updateChannel(Channel *channel) {
  auto fd = channel->getFd();
  auto st = channel->getStatus();
  // auto ev = channel->getEvents();
  // channel->debugEvents();
  if (st == Channel::INIT || st == Channel::DELETED) {
    /* 如果状态是初始化或者已经被删除了，我们add 该 Channel */
    update(EPOLL_CTL_ADD, channel);
    channel->setStatus(Channel::LISTEN);
    channels_[fd] = channel;
  } else if (st == Channel::LISTEN) {
    /* 如果状态是监听中，
      但是没有我们任何需要监听的读写事件 ，那就DEL*/
    if (channel->noEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setStatus(Channel::NOT_LISTEN);
    } else {
      /*    但是有我们任何需要监听的读写事件 ，那就MOD*/
      update(EPOLL_CTL_MOD, channel);
    }
  } else if (st == Channel::NOT_LISTEN) {
    /* 如果没有监听且事件并且有事件 我们就ADD */
    if (!channel->noEvent()) {
      update(EPOLL_CTL_ADD, channel);
      channel->setStatus(Channel::LISTEN);
    }
  }
}

void Epoller::removeChannel(Channel *channel) {
  auto fd = channel->getFd();
  auto st = channel->getStatus();
  // auto ev = channel->getEvents();
  auto ret = channels_.erase(fd);
  if (ret == -1) {
    LOG(DEBUG) << "channel erase error" << adl::endl;
  }
  /* 如果Channel监听状态 我们就DEL该channel */
  if (st == Channel::LISTEN) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->setStatus(Channel::DELETED);
}

bool Epoller::hasChannel(Channel *channel) const {
  ChannelMap::const_iterator it = channels_.find(channel->getFd());
  return it != channels_.end() && it->second == channel;
}

/* 将一次Epoll_wait后活跃的通道 添加到活跃通道列表中 */
void Epoller::fillActiveChannels(int numEvents,
                                 ChannelList *activeChannels) const {
  assert(implicit_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; ++i) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
    /* 通道记录返回的事件类型 */
    channel->setRevents(events_[i].events);
    /* 活跃的通道中加入该通道 */
    activeChannels->push_back(channel);
  }
}

/* epoll_ctl */
void Epoller::update(int operation, Channel *channel) {
  struct epoll_event event;
  explicit_bzero(&event, sizeof event);
  event.events = channel->getEvents();
  event.data.ptr = channel;
  int fd = channel->getFd();

  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    LOG(ERROR) << "Epoller::update epoll_ctl the socket fd is " << fd
               << "and the event is "
               << (event.events & EPOLLIN ? "EPOLLIN" : " ")
               << (event.events & EPOLLOUT ? "EPOLLOUT" : " ") << adl::endl;
  }
}

void Epoller::assertInLoopThread() const { ownerLoop_->assertInLoopThread(); }
