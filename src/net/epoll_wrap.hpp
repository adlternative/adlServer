#ifndef EPOLL_WRAP_HPP
#define EPOLL_WRAP_HPP
#include <cerrno>
#include <sys/epoll.h>
#include <system_error>

class epoll_wrapper {

protected:
  static constexpr int INVALID_FD = -1;

  int epfd_ = INVALID_FD;

  epoll_wrapper() : epfd_(epoll_create1(EPOLL_CLOEXEC)) {
    if (epfd_ == -1)
      throw std::system_error{errno, std::system_category(), "epoll_create1"};
  }
  /**
   * @brief ctl
   * 会抛出异常，因此我们需要在外部接住 system_error
   * @param fd 事件描述符
   * @param opt EPOLL_CTL_*
   * @param ep_evt epoll事件结构体
   */
  virtual void ctl(int fd, int opt, struct epoll_event &ep_evt) {
    int ret;

    if (fd == epfd_)
      throw std::invalid_argument{"epoll_ctl: fd == epfd_"};

    for (;;) {
      ret = epoll_ctl(epfd_, opt, fd, &ep_evt);
      if (!ret)
        return;
      if (errno != EEXIST) {
        /*
        EPERM 目标文件fd不支持epoll。这个错误可以例如，
        如果fd指代一个常规文件或一个
        目录。

        ENOSPC 尝试注册 ( EPOLL_CTL_ADD )
        时遇到/proc/sys/fs/epoll/max_user_watches施加的限制 epoll
        实例上的新文件描述符。见epoll(7) 了解更多详情。

        ENOENT op是 EPOLL_CTL_MOD 或 EPOLL_CTL_DEL，而fd不是注册到这个 epoll
        实例。

        ELOOP fd指的是一个 epoll 实例 这个EPOLL_CTL_ADD
        操作会导致循环的epoll实例相互监视 或嵌套深度大于 5。

        EEXIST 操作是EPOLL_CTL_ADD,
        但是指定的文件描述符已经在被注册在epoll实例中。

        EBADF epfd 或者 fd 不是有效的文件描述符

        EINVAL
        1. epfd 不是 epoll的文件描述符，或者fd == epollfd, 或者请求的操作
        op 不被该接口支持。
        2. 一个非法的事件类型与 EPOLLEXCLUSIVE 被指定
        3. op 是 EPOLL_CTL_MOD 并且 events 包含 EPOLLEXCLUSIVE.
        4. op 是 EPOLL_CTL_MOD 并且 EPOLLEXCLUSIVE 之前应用到这个 <epfd, fd> 对.
        */
        /* log/throw */
        throw std::system_error{errno, std::system_category(), "epoll_ctl"};
      }
      /* EEXIST 则 MOD 重试 */
      opt = EPOLL_CTL_MOD;
    }
  }

  virtual void add(int fd, struct epoll_event &ep_evt) {
    ctl(fd, EPOLL_CTL_ADD, ep_evt);
  }

  virtual void mod(int fd, struct epoll_event &ep_evt) {
    ctl(fd, EPOLL_CTL_MOD, ep_evt);
  }

  virtual void del(int fd) {
    /* 内核版本2.6.9 之前不允许空指针 */
    struct epoll_event ep_evt {};
    ctl(fd, EPOLL_CTL_DEL, ep_evt);
  }

  virtual int loop(struct epoll_event *__events, int __maxevents, int wait_ms) {
    int count = 0;
    do
      count = epoll_wait(epfd_, __events, __maxevents, wait_ms);
    while (count == -1 && errno == EINTR);
    if (count == -1)
      throw std::system_error{errno, std::system_category(), "epoll_wait"};

    return count;
  }
};

#endif