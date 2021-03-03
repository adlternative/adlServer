#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include "../include/headFile.h"
#include "../net/netBuffer.h"
#include <map>

namespace adl {
namespace http {

enum Method {
  GET = 0,
  POST,
  // HEAD,
  // PUT,
  // PATCH,
  // DELETE,
  // TRACE,
  // CONNECT,
  DONT_UNDERSTAND,
};

/* 请求方法 */
class httpRequestMethod {
public:
  httpRequestMethod() = default;
  void setMethod(Method method) { method_ = method; }
  Method method_;
  static const char *methodString[DONT_UNDERSTAND];
};

/* 请求行? */
class httpRequestLine {
public:
  httpRequestLine() = default;
  void setVersion(string v) { version_ = v; }
  void setUrl(string u) { url_ = u; }
  void setMethod(Method method) { method_.method_ = method; }
  string getUrl() { return url_; }
  string getVersion() { return version_; }
  Method getMethod() { return method_.method_; }

  void debug();

private:
  string version_;           /* 版本号 1.0 1.1 */
  string url_;               /* / /a.txt */
  httpRequestMethod method_; /* GET POST ...*/
};

/* 请求头 */
class httpRequestHead {
public:
  httpRequestHead() = default;
  void add(string k, string v) { headMap_.insert({k, v}); }
  void remove(string k) {
    if (-1 == headMap_.erase(k)) {
      // LOG
    }
  }
  string getValue(string k) {
    string s;
    auto it = headMap_.find(k);
    if (it != headMap_.end())
      return it->second;
    return s;
  }
  std::pair<string, string> get(string k) {
    auto it = headMap_.find(k);
    if (it == headMap_.end())
      return std::pair<string, string>();
    return *it;
  }
  void debug() {
    LOG(DEBUG) << "HeadMap: " << adl::endl;
    for_each(headMap_.begin(), headMap_.end(),
             [](const std::pair<string, string> &ss) {
               LOG(DEBUG) << "key: " << ss.first << " value: " << ss.second
                          << adl::endl;
             });
  }

private:
  std::map<string, string> headMap_;
};

/* 请求消息体 */
struct httpRequestMessageBody {
public:
  void setMessage(const char *message, size_t len) {
    message_ = string(message, len);
  }
  string message_;
};

/* 请求 */
struct httpRequest {
public:
  httpRequest() : parseSize_(0) {}
  void debug() {
    line_.debug();
    head_.debug();
  }
  int parseSize_;
  httpRequestLine line_;
  httpRequestHead head_;
  httpRequestMessageBody body_;
};

class httpParser : boost::noncopyable {
public:
  /* 总状态机 */
  enum FinnalState {
    Success_,    /* 成功解析 */
    Failed_,     /* 解析失败 */
    Incomplete_, /* 不完整 需要重新等待解析 */
  };

  /* 行状态机 */
  enum LineState {
    LineOk_,         /* 行成功解析 */
    LineBad_,        /* 行解析失败 */
    LineIncomplete_, /* 行不完整 需要重新等待解析 */
  };

  /* 看我们解析到结构的哪里了 */
  enum ProcessState { Line_, Head_, Body_ };

  /* 请求行状态机 */
  enum RequestLineState { Method_, Url_, Version_ };
  httpParser(const char *buf, int len)
      : buf_(buf), len_(len), cur_(0), l_beg_(0), l_end_(0),
        processState_(Line_), finalState_(Incomplete_),
        lineState_(LineIncomplete_), request_(new httpRequest()) {}

  void requestLineParse();
  void requestHeadParse();
  void requestBodyParse(int len);
  std::shared_ptr<httpRequest> getRequest() { return request_; }
  FinnalState parse();

  // ProcessState

private:
  const char *CR_LF = "\r\n";

  FinnalState finalState_;
  LineState lineState_;
  ProcessState processState_;

  std::shared_ptr<httpRequest> request_; /* 我将解析后的内容放在这里 */

  int l_beg_; /* 标记当前行开始 */
  int l_end_; /* 标记当前行开始 */
  int cur_;   /* 游标, 表示当前指向的位置 */
  const char *buf_;
  int len_; /* 数据总长度 */
};
} // namespace http
} // namespace adl

#endif