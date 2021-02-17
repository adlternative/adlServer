#include "Epoller.h"
#include "../util.h"
#include "Channel.h"
#include <cassert>
using namespace adl;

Epoller::Epoller(EventLoop *loop)
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(0), ownerLoop_(loop) {
  if (epollfd_ == -1) {
    // epollfd_ 错误处理
  }
}

Epoller::~Epoller() { ::close(epollfd_); }

/* epoll_wait */
timeStamp Epoller::poll(int timeoutMs, ChannelList *activeChannels) {
  /* Log */
  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                               static_cast<int>(events_.size()), timeoutMs);
  timeStamp now(timeStamp::now());
  int savedErrno = errno; /* 保存当前错误 */
  if (numEvents < 0) {
    /* LOG ERROR */
  } else if (numEvents == 0) {
    /* 如果我们设置了超时时间，
    并且在这段时间内没有事件发生 */
    /* LOG TRACE */
  } else {
    fillActiveChannels(numEvents, activeChannels); /* 填充活跃的通道 */
    if (implicit_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  }
  return now;
}

void Epoller::updateChannel(Channel *channel) {
  auto fd = channel->getFd();
  auto st = channel->getStatus();
  auto ev = channel->getEvents();
  if (st == Channel::INIT || st == Channel::DELETED) {
    update(EPOLL_CTL_ADD, channel);
    channel->setStatus(Channel::LISTEN);
    channels_[fd] = channel;
  } else if (st == Channel::LISTEN) {
    if (channel->noEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setStatus(Channel::NOT_LISTEN);
    } else
      update(EPOLL_CTL_MOD, channel);
  } else if (st == Channel::NOT_LISTEN) {
    if (!channel->noEvent()) {
      update(EPOLL_CTL_ADD, channel);
      channel->setStatus(Channel::LISTEN);
    }
  }
}

void Epoller::removeChannel(Channel *channel) {
  auto fd = channel->getFd();
  auto st = channel->getStatus();
  auto ev = channel->getEvents();
  auto ret = channels_.erase(fd);
  if (ret == -1) {
    /* LOG */
  }
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
  bzero(&event, sizeof event);
  event.events = channel->getEvents();
  event.data.ptr = channel;
  int fd = channel->getFd();
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    /* log */
  }
}

void Epoller::assertInLoopThread() const { ownerLoop_->assertInLoopThread(); }
