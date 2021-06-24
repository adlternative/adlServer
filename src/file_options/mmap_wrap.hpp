#ifndef MMAP_WRAP_HPP
#define MMAP_WRAP_HPP
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace adl {
namespace file_options {
/**
 * @brief 只读的mmap 主要我们可以通过 data() 快速获取文件的数据
 */
class mmap {
public:
  /**
   * @brief Construct a new mmap object
   * valgrind 无内存泄漏问题 尽管在构造函数抛出异常不好，
   * 但是这个它在当前的环境下会执行 clean_up 来释放资源，
   * 因此没有太大的问题，如果愿意可以用二段初始化的方式。
   *
   * @param path 文件路径
   */
  mmap(const char *path) : fd_(-1), size_(0), addr_(MAP_FAILED) {
    if (addr_ == MAP_FAILED) {
      fd_ = open(path, O_RDONLY);
      if (fd_ == -1) {
        throw std::runtime_error(string(path) + " : " + strerror(errno));
      }

      struct stat sb;
      if (fstat(fd_, &sb) == -1) {
        cleanup();
        throw std::runtime_error(string(path) + " : " + strerror(errno));
      }
      size_ = sb.st_size;
      /* 只读打开文件 */
      addr_ = ::mmap(NULL, size_, PROT_READ, MAP_PRIVATE, fd_, 0);

      if (addr_ == MAP_FAILED) {
        cleanup();
        throw std::runtime_error(string(path) + " : " + strerror(errno));
      }
    }
  }
  ~mmap() { cleanup(); }
  /**
   * @brief 文件是否打开成功
   *
   * @return size_t
   */
  bool is_open() const { return addr_ != MAP_FAILED; }
  /**
   * @brief 文件大小
   *
   * @return size_t
   */
  size_t size() const { return size_; }
  /**
   * @brief c风格文件数据
   *
   * @return const char*
   */
  const char *data() const { return (const char *)addr_; }

private:
  void cleanup() {
    if (addr_ != MAP_FAILED) {
      munmap(addr_, size_);
      addr_ = MAP_FAILED;
    }

    if (fd_ != -1) {
      close(fd_);
      fd_ = -1;
    }
    size_ = 0;
  }

  int fd_;
  size_t size_;
  void *addr_;
};
} // namespace file_options
} // namespace adl
#endif
