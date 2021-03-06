#include "netBuffer.h"
#include "../log/fileUtil.h"
#include "Socket.h"
/* QUE:WHY ET more effective to LT */

namespace adl { // namespace adl

/* 注意这里主要是读非阻塞的 socket 的操作 */
/* 添加了读文件的操作用于sendFile从指定偏移量开始发送，
所以使用pread保证原子 */
int netBuffer::readFd(int fd, int *savedErrno, bool *writeClosed,
                      int sendFileOffset) {
  LOG(TRACE) << "netBuffer::readFd" << adl::endl;

  int getSize = 0;
  int AllSize = 0;

  for (;;) {

    if (sendFileOffset > 0)
      getSize = adl::sock::pread(fd, buf_ + w_, writeable(),
                                 sendFileOffset + getSize);
    else
      getSize = adl::sock::read(fd, buf_ + w_, writeable());
    /* 出错处理 */
    if (getSize < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return AllSize;
      } else {
        *savedErrno = errno;
        return -1;
      }
    } else if (getSize == 0) {
      *writeClosed = true;
      return getSize; /* 对方关闭连接 ，咋办？ */
    } else {
      /* 读到多少，w_加多少 */
      LOG(DEBUG) << "readSize: " << getSize << adl::endl;
      w_ += getSize;
      AllSize += getSize;
      /* 如果可写的byte太少了，扩容 */
      if (writeable() <= 0.2 * alloc_) {
        checkCapcity(4096);
      }
    }
  }
  return AllSize;
}

} // namespace adl