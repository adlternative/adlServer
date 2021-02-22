#include "netBuffer.h"
#include "../log/fileUtil.h"
#include "Socket.h"
/* QUE:WHY ET more effective to LT */

namespace adl { // namespace adl

int netBuffer::readFd(int fd, int *savedErrno, bool *closed) {
  int getSize = 0;
  int AllSize = 0;
  for (;;) {
    getSize = adl::sock::read(fd, buf_, writeable());
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
      *closed = 1;
      return getSize; /* 对方关闭连接 ，咋办？ */
    } else {
      /* 读到多少，w_加多少 */
      retrieve(getSize);
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