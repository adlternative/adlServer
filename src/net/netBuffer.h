#ifndef NETBUFFER_H
#define NETBUFFER_H
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <stddef.h>
#include <string>
#include <vector>
namespace adl {
class Buffer {
public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;
  /* 设置初始化大小 1024 */
  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(kCheapPrepend + initialSize), /* 初始化大小1024+8 */
        readerIndex_(kCheapPrepend),          /* r w 初始于8B */
        writerIndex_(kCheapPrepend) {
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == kCheapPrepend);
  }

  // implicit copy-ctor, move-ctor, dtor and assignment are fine
  // NOTE: implicit move-ctor is added in g++ 4.6

  void swap(Buffer &rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }
  /* 可读的字节w-r */
  size_t readableBytes() const { return writerIndex_ - readerIndex_; }
  /* 可写的字节bufsize-w*/
  size_t writableBytes() const { return buffer_.size() - writerIndex_; }
  /* prependsize与r相同 */
  size_t prependableBytes() const { return readerIndex_; }

  /* peek从r开始看 */
  const char *peek() const { return begin() + readerIndex_; }

  /* 寻找\r\n */
  const char *findCRLF() const {
    // FIXME: replace with memmem()?
    const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    /* 都到w了说明没有找到 */
    return crlf == beginWrite() ? NULL : crlf;
  }

  const char *findCRLF(const char *start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    // FIXME: replace with memmem()?
    const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? NULL : crlf;
  }

  /* 寻找\n */
  const char *findEOL() const {
    const void *eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char *>(eol);
  }

  const char *findEOL(const char *start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void *eol = memchr(start, '\n', beginWrite() - start);
    return static_cast<const char *>(eol);
  }

  /* 取走，其实就是我们使用完了以后将r+len */
  void retrieve(size_t len) {
    /* 如果取走的大小len<=w-r ; r+=len;*/
    assert(len <= readableBytes());
    if (len < readableBytes()) {
      readerIndex_ += len;
    } else {
      retrieveAll();
    }
  }
  /* 取走，直到某个字符 */
  void retrieveUntil(const char *end) {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
  }
  /* 取出8B */
  void retrieveInt64() { retrieve(sizeof(int64_t)); }
  /* 取出4B */
  void retrieveInt32() { retrieve(sizeof(int32_t)); }
  /* 取出2B */
  void retrieveInt16() { retrieve(sizeof(int16_t)); }
  /* 取出1B */
  void retrieveInt8() { retrieve(sizeof(int8_t)); }

  /* 字面意思：取出所有...但是这里应该是r,w重置为8 (我们读走所有buffer之后)*/
  void retrieveAll() {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
  }
  /* 取走所有内容到String中 */
  std::string retrieveAllAsString() {
    return retrieveAsString(readableBytes());
  }
  /* 取走所有内容到String中 */
  std::string retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
  }

  void append(const char * /*restrict*/ data, size_t len) {
    /* 少则扩容 */
    ensureWritableBytes(len);
    /* copy,可以看到很奇妙，
      beginWrite是buffer的某个位置的字符的指针，
      但因为vector<char>中每个字符的地址都是连续的，
      因此copy可以到正常的位置。
    */
    std::copy(data, data + len, beginWrite());
    /* w+=len */
    hasWritten(len);
  }
  void append(const void * /*restrict*/ data, size_t len) {
    append(static_cast<const char *>(data), len);
  }
  /* 少则扩容 */
  void ensureWritableBytes(size_t len) {
    // buf.size - w <len
    if (writableBytes() < len) {
      makeSpace(len);
    }
    assert(writableBytes() >= len);
  }

  /* 返回开始写的字节 */
  char *beginWrite() { return begin() + writerIndex_; }

  /* 返回开始写的字节，const 版本只能 读 */
  const char *beginWrite() const { return begin() + writerIndex_; }

  /* 我们append 之后 w +=len */
  void hasWritten(size_t len) {
    assert(len <= writableBytes());
    writerIndex_ += len;
  }
  /* w-=len */
  void unwrite(size_t len) {
    assert(len <= readableBytes());
    writerIndex_ -= len;
  }
  void appendInt32(int32_t x) {
    int32_t be32 = htonl(x);
    append(&be32, sizeof be32);
  }

  void appendInt16(int16_t x) {
    int16_t be16 = htons(x);
    append(&be16, sizeof be16);
  }

  void appendInt8(int8_t x) { append(&x, sizeof x); }

  int32_t readInt32() {
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
  }

  int16_t readInt16() {
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
  }

  int8_t readInt8() {
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
  }

  int32_t peekInt32() const {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof be32);
    return ntohl(be32);
  }

  int16_t peekInt16() const {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof be16);
    return ntohs(be16);
  }

  int8_t peekInt8() const {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
  }

  void prependInt32(int32_t x) {
    int32_t be32 = htonl(x);
    prepend(&be32, sizeof be32);
  }

  void prependInt16(int16_t x) {
    int16_t be16 = htons(x);
    prepend(&be16, sizeof be16);
  }

  void prependInt8(int8_t x) { prepend(&x, sizeof x); }
  /* 向前写东西 */
  void prepend(const void * /*restrict*/ data, size_t len) {
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    const char *d = static_cast<const char *>(data);
    std::copy(d, d + len, begin() + readerIndex_);
  }
  /*调整大小  */
  void shrink(size_t reserve) {
    /* 可写确保有reserve大小 */
    ensureWritableBytes(reserve);
    /* resize to */
    buffer_.shrink_to_fit();
  }

  size_t internalCapacity() const { return buffer_.capacity(); }

  /// Read data directly into buffer.
  ///
  /// It may implement with readv(2)
  /// @return result of read(2), @c errno is saved
  ssize_t readFd(int fd, int *savedErrno);

private:
  char *begin() { return &*buffer_.begin(); }

  const char *begin() const { return &*buffer_.begin(); }

  /* 当writableBytes()<len 我们才会调用makeSpace ，
     然而如果writableBytes() + prependableBytes() < len + kCheapPrepend
    说明pre>kChecPrepend 所以可以移动[r,w]到 [kCheapPrepend:kCheapPrepend+w-r]
  */
  void makeSpace(size_t len) {
    // 这种情况下就直接resize扩容
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
      // FIXME: move readable data
      buffer_.resize(writerIndex_ + len);
    } else {
      // move readable data to the front, make space inside buffer
      assert(kCheapPrepend < readerIndex_);
      size_t readable = readableBytes();
      std::copy(begin() + readerIndex_, begin() + writerIndex_,
                begin() + kCheapPrepend);
      readerIndex_ = kCheapPrepend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == readableBytes());
    }
  }

private:
  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;

  static const char kCRLF[];
};

} // namespace adl
#endif
