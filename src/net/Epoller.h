#ifndef EPOLLER_H
#define EPOLLER_H
#include "../base/timeStamp.h"
#include "Channel.h"
#include "EventLoop.h"
#include <boost/noncopyable.hpp>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <vector>
namespace adl {
class Epoller : boost::noncopyable {
public:
  typedef std::vector<Channel *> ChannelList;

  Epoller(const std::shared_ptr<EventLoop> &loop);

  ~Epoller();
  timeStamp poll(int timeoutMs, ChannelList *activeChannels);
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel) const;

private:
  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
  void update(int operation, Channel *channel);
  void assertInLoopThread() const;
  typedef std::map<int, Channel *> ChannelMap; /* {fd,Channel} */
  ChannelMap channels_;                        /* {fd,Channel}字典 */

  int epollfd_;

  using EventList = std::vector<struct epoll_event>;
  EventList events_;                   /* Epoller上的监听列表 */
  std::weak_ptr<EventLoop> ownerLoop_; /* 拥有本 Epoller 的EventLoop */
};
} // namespace adl
#endif
