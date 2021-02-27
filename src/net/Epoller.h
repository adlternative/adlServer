#ifndef EPOLLER_H
#define EPOLLER_H
#include "../log/timeStamp.h"
#include "Channel.h"
#include "EventLoop.h"
#include <boost/noncopyable.hpp>
#include <map>
#include <sys/epoll.h>
#include <vector>
namespace adl {
class Epoller : boost::noncopyable {
public:
  typedef std::vector<Channel *> ChannelList;

  Epoller(EventLoop *loop);

  ~Epoller();
  timeStamp poll(int timeoutMs, ChannelList *activeChannels);
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel) const;

private:
  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
  void update(int operation, Channel *channel);
  void assertInLoopThread() const ;
  typedef std::map<int, Channel *> ChannelMap; /* {fd,Channel} */
  ChannelMap channels_;                        /* {fd,Channel}字典 */

  int epollfd_;

  typedef std::vector<struct epoll_event> EventList;
  EventList events_;     /* Epoller上的监听列表 */
  EventLoop *ownerLoop_; /* 拥有本 Epoller 的EventLoop */
};
} // namespace adl
#endif
