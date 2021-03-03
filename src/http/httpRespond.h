#ifndef HTTPREPOND_H
#define HTTPREPOND_H
#include "../include/headFile.h"
#include "../net/tcpConnection.h"

#include "./httpParser.h"
namespace adl {
namespace http {

class httpResPondLine {
public:
private:
  // const char *respondLine[2] = {"200 OK", "404 Not Found"};
};
class httpResPondHead {
public:
private:
  /* map */
};
class httpResPondMessageBody {
public:
private:
  /* sp here should have a source map?  */
  /* . .. ->404 */
  /* / -> source/index.html */
  /* /a.txt -> source/a.txt */
  /* how to sendFile
    append?sendfile?
    如果出现写缓冲区满了?需要等会再sendfile还是append?
    文件损坏或删除？文件偏移量？
  */
  /*
    400 Bad Request;
 */
};

class httpRespond {
public:
  enum repState_ { _200Ok, _400BadRequest, _404NotFound, DontKnow };
  static const char *repString[DontKnow];
  httpRespond(const TcpConnectionPtr &conn) : fd_(-1), size_(0), conn_(conn) {}
  void setStatusCode(repState_ state) { state_ = state; }
  void setVersion(string v) { version_ = v; }
  void setBody(string &&body) {
    body_ = std::move(body);
    addContentSize(body_.size());
  }
  void setSourceDir(string dir = "./source/") { sourceDir_ = dir; }
  void setSourceFile(string file) { sourceFile_ = file; }
  void addContentType(string type) { addHeader("Content-Type", type); }
  void addContentSize(int size) {
    size_ = size;
    addHeader("Content-Length", std::to_string(size));
  }
  void addKeepAlive(bool on) {
    if (on)
      addHeader("Connection", "keep-alive");
    else
      addHeader("Connection", "Close");
  }
  void addHeader(const string &key, const string &value) {
    headMap_[key] = value;
  }
  void linkFileFd(int fd) {
    if (fd < 0) {
      // LOG(ERROR)
    }
    fd_ = fd;
  }
  void respond() {
    /* version */
    /* stateCode */
    std::stringstream stream;
    stream << version_ << " " << repString[state_] << "\r\n";
    /* head */
    for (auto &&pai : headMap_) {
      stream << pai.first << ": " << pai.second << "\r\n";
    }
    stream << "\r\n";
    string line_head = stream.str();
    LOG(DEBUG) << line_head << adl::endl;
    conn_->send(line_head.c_str(), line_head.size());
    /* body */
    if (fd_ > 0 && size_ > 0) {
      conn_->sendFile(fd_, size_);
    } else if (body_.size() == size_) {
      conn_->send(body_.c_str(), size_);
    }
  }

  int size_;        /* bodysize */
  repState_ state_; /*  */
  std::map<string, string> headMap_;
  string version_;
  string sourceDir_;  /* ./source */
  string sourceFile_; /* 我们需要发送的文件 */
  std::shared_ptr<httpRequest> request_;
  TcpConnectionPtr conn_;
  int fd_;      /* 需要发送的文件fd */
  string body_; /* 如果发送的不是文件，而只是一个body */
};
} // namespace http
} // namespace adl
#endif
