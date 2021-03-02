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

private:
  std::map<string, string> headMap_;
};

/* 请求消息体 */
class httpRequestMessageBody {};

/* 请求 */
class httpRequest {
public:
  httpRequest() = default;
  void debug() { line_.debug(); }
  httpRequestLine line_;
  httpRequestHead head_;
  httpRequestMessageBody body_;
};

class httpParser : boost::noncopyable {
public:
  /* 总状态机 */
  enum FinnalState {
    Failed_,     /* 解析失败 */
    Incomplete_, /* 不完整 需要重新等待解析 */
    Success_     /* 成功解析 */
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

  LineState requestLineParse() {
    RequestLineState rlState = Method_;
    l_end_ = cur_;
    cur_ = l_beg_;
    int len = l_end_ - l_beg_;
    /* 解析Method_ */
    for (; cur_ != l_end_;) {
      if (rlState == Method_) {
        ////DEBUG_("parse\n");
        if (len >= strlen("GET") && starts_with(buf_ + l_beg_, "GET")) {
          // ERR_("%d %d %d\n", len, l_beg_, l_end_);/* 16 0 16 */
          ////DEBUG_("parse\n");
          request_->line_.setMethod(GET);
          // DEBUG_("parse\n");
          cur_ += strlen("GET");
          // DEBUG_("parse\n");
        } else if (len >= strlen("POST") &&
                   starts_with(buf_ + l_beg_, "POST")) {
          request_->line_.setMethod(POST);
          cur_ += strlen("POST");
        } else {
          if (starts_with(buf_ + l_beg_, "HEAD") &&
              starts_with(buf_ + l_beg_, "PATCH")) {
            /* 不支持的方法，返回404? */
            // LOG()
            lineState_ = LineBad_;
            break;
          } else {
            /* 错误的请求！ */
            // LOG()
            lineState_ = LineBad_;
            break;
          }
        }
        /* 空格 */
        if (cur_ == l_end_)
          break;
        else if (buf_[cur_++] != ' ') {
          break;
        }
        rlState = Url_;
        continue;
      } else if (rlState == Url_) {
        // DEBUG_("parse\n");
        /* 寻找下一个空格 */
        int uBegin = cur_;
        while (cur_ != l_end_ && buf_[cur_++] != ' ')
          continue;
        if (cur_ == l_end_)
          break;
        request_->line_.setUrl(string(buf_ + uBegin, cur_ - uBegin));
        if (cur_ == l_end_)
          break;
        else if (buf_[cur_++] != ' ') {
          break;
        }
        rlState = Version_;
      } else if (rlState == Version_) {
        DEBUG_("parse\n");
        printf("version:%s\n", buf_ + cur_);

        if (!strncmp(buf_ + cur_, "HTTP/1.1", l_end_ - cur_) ||
            !strncmp(buf_ + cur_, "HTTP/1.0", l_end_ - cur_)) {
          cur_ = l_end_;
          lineState_ = LineOk_;
          break;
        } else {
          cur_ = l_end_;
          lineState_ = LineBad_;
          break;
        }
      }
    }
    request_->debug();

    return lineState_;
  }
  LineState requestHeadParse() { return LineBad_; }
  LineState requestBodyParse() { return LineBad_; }
  std::shared_ptr<httpRequest> getRequest() { return request_; }
  FinnalState parse() {
    for (; cur_ != len_;) {
      lineState_ = LineIncomplete_;
      /* 寻找本行的\r */
      while (cur_ != len_ && buf_[cur_++] != '\r')
        continue;
      /* 如果 没有遇到 \r 而是遇上结束符号 我们就退出*/
      if (cur_ == len_)
        break;
      /* 那么继续找\n */
      if (cur_ != len_ && buf_[cur_++] == '\n') {
        /* get A line  */
        /* parser from begin to end  */
        if (processState_ == Line_) {
          lineState_ = requestLineParse();
          if (lineState_ == LineBad_) {
            /* 404 */
            /*  */
          } else if (lineState_ == LineOk_) {
            /* 继续 */
            processState_ = Head_;
            continue;
          } else {
            finalState_ = Incomplete_;
            /* 不完整就退出本轮解析  */
            break;
          }
        } else if (processState_ == Head_) {
          // DEBUG_("parse\n");
          // requestLineParse();
        } else if (processState_ == Body_) {
          // DEBUG_("parse\n");
          // requestLineParse();
        }
      }
      /* 如果没找到\n */
      continue;
    }
    return finalState_;
  }

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
  int len_;
};
} // namespace http
} // namespace adl

#endif