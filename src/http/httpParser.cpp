#include "./httpParser.h"
namespace adl {
namespace http {
/* post , get, head,delete, put, connect, options, patch,
 * propfind, propatch,
 * mkcol, copy, move, lock, unlock, trace, head */
const char *httpRequestMethod::methodString[DONT_UNDERSTAND] = {
    "GET", "POST" /* , "HEAD", "PUT", "PATCH", "DELETE", "TRACE", "CONNECT", */
};

void httpRequestLine::debug() {
  LOG(DEBUG) << "RequestLine: " << adl::endl;
  LOG(DEBUG) << "version_: " << version_ << " url_: " << url_
             << " method: " << httpRequestMethod::methodString[method_.method_]
             << adl::endl;
}

void httpParser::requestLineParse() {

  int len;
  RequestLineState rlState;

  for (; cur_ != len_;) {
    /* 寻找本行的\r */
    if (buf_[cur_++] != '\r')
      continue;
    /* 有\r */
    /* 到达结尾 */
    if (cur_ == len_) {
      /* 不完整 */
      lineState_ = LineIncomplete_;
      goto finished1;
    }
    /* 那么继续找 \n */
    if (buf_[cur_++] == '\n')
      break;
  }

  l_end_ = cur_;
  cur_ = l_beg_;
  len = l_end_ - l_beg_;
  rlState = Method_;
  /* 解析Method_ */
  for (; cur_ != l_end_;) {
    if (rlState == Method_) {
      if (len >= strlen("GET") && starts_with(buf_ + l_beg_, "GET")) {
        // ERR_("%d %d %d\n", len, l_beg_, l_end_);/* 16 0 16 */
        request_->line_.setMethod(GET);
        cur_ += strlen("GET");
      } else if (len >= strlen("POST") && starts_with(buf_ + l_beg_, "POST")) {
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
      /* 寻找下一个空格 */
      int uBegin = cur_;
      while (cur_ != l_end_ && buf_[cur_++] != ' ')
        continue;
      if (cur_ == l_end_)
        break;
      request_->line_.setUrl(string(buf_ + uBegin, cur_ - uBegin - 1));
      LOG(DEBUG) << buf_[uBegin] << cur_ - uBegin << adl::endl;
      if (cur_ == l_end_) {
        break;
      }
      rlState = Version_;
    } else if (rlState == Version_) {
      if (!strncmp(buf_ + cur_, "HTTP/1.1", 8) ||
          !strncmp(buf_ + cur_, "HTTP/1.0", 8)) {
        request_->line_.setVersion(string(buf_ + cur_, 8));
        cur_ = l_end_; /* 置于下一行行首 */
        lineState_ = LineOk_;
        break;
      } else {
        cur_ = l_end_;
        lineState_ = LineBad_;
        break;
      }
    }
  }
  // request_->debug();
finished1:
  return;
}

void httpParser::requestHeadParse() {
  /* 下面要开始寻找\r\n了，
    找到了前面就是一行
    找到空行的\r\n就退出ok */

  for (; cur_ != len_;) {
    cur_ = l_beg_ = l_end_;
    lineState_ = LineIncomplete_;
    while (cur_ != len_ && buf_[cur_++] != '\r')
      continue;
    /* 否则如果到末尾了都找不到\r 退出for */
    if (cur_ == len_) {
      break;
    }
    /* 找不到\n 继续找*/
    if (buf_[cur_++] != '\n')
      continue;
    l_end_ = cur_; /* 下一行开头 */
                   /* 如果找到了\n */
                   /* get a line  */

    /* 无内容行成功退出 */
    if (l_beg_ + 2 == l_end_) {
      lineState_ = LineOk_;
      break;
    }
    /* 寻找空格 */
    for (cur_ = l_beg_; cur_ < l_end_; cur_++) {
      if (buf_[cur_] == ' ')
        break;
    }
    /* 没有空格 错误退出 */
    if (cur_ == l_end_) {
      lineState_ = LineBad_;
      break;
    }
    /* 有空格 */
    /* set {k,v} */
    // LOG(DEBUG) << "k :" << buf_len(buf_ + l_beg_, cur_ - 1 - l_beg_)
    //           << "\tv :" << buf_len(buf_ + cur_ + 1, l_end_ - 3 - l_beg_)
    //           << adl::endl;
    request_->head_.add(string(buf_ + l_beg_, buf_ + cur_ - 1),
                        string(buf_ + cur_ + 1, buf_ + l_end_ - 2));
    /* 本行ok */
    lineState_ = LineOk_;
    /* 然后下一行 */
  }

  return;
}

void httpParser::requestBodyParse(int len) {
  if (cur_ + len > len_) {
    lineState_ = LineIncomplete_;
    return;
  } else if (cur_ + len <= len_) {
    request_->body_.setMessage(buf_ + cur_, len);
    cur_ += len;
    lineState_ = LineOk_;
    return;
  }
}

httpParser::FinnalState httpParser::parse() {
  for (; cur_ != len_;) {
    lineState_ = LineIncomplete_;

    /* get A line  */
    /* parser from begin to end  */
    if (processState_ == Line_) {
      LOG(TRACE) << "Parse Request Line" << adl::endl;
      requestLineParse();
      if (lineState_ == LineBad_) {
        finalState_ = Failed_;
        LOG(TRACE) << "Bad Request Line" << adl::endl;
        goto finished;
      } else if (lineState_ == LineOk_) {
        LOG(INFO) << "Good Request Line" << adl::endl;
        /* 继续 */
        processState_ = Head_;
        continue;
      } else {
        LOG(INFO) << "Incomplete Request Line, continue Waiting..."
                  << adl::endl;
        finalState_ = Incomplete_;
        /* 不完整就退出本轮解析  */
        break;
      }
    } else if (processState_ == Head_) {
      LOG(TRACE) << "Parse Request Head" << adl::endl;
      requestHeadParse();
      if (lineState_ == LineBad_) {
        finalState_ = Failed_;
        LOG(TRACE) << "Bad Request Head" << adl::endl;
        goto finished;
        /* 404 */
      } else if (lineState_ == LineOk_) {
        LOG(INFO) << "Good Request Head" << adl::endl;
        /* 继续 */
        if (request_->line_.getMethod() == GET) {
          /* 这时候应该就是没有内容才对 */
          request_->parseSize_ = cur_;
          finalState_ = Success_;
          break;
        }
        processState_ = Body_;
        // request_->debug();
        continue;
      } else {
        LOG(INFO) << "Incomplete Request Head, continue Waiting..."
                  << adl::endl;
        finalState_ = Incomplete_;
        /* 不完整就退出本轮解析  */
        break;
      }
    } else if (processState_ == Body_) {
      /* post */
      auto kv = request_->head_.get("Content-Length");
      if (request_->line_.getMethod() == POST && !kv.first.empty() &&
          cur_ < len_) {
        int value = std::stoi(kv.second);
        if (value > 0) {
          requestBodyParse(value);
          if (lineState_ == LineBad_) {
            LOG(TRACE) << "Bad Request Body" << adl::endl;
            finalState_ = Failed_;
            goto finished;
            /* 404 */
          } else if (lineState_ == LineOk_) {
            LOG(INFO) << "Good Request Body" << adl::endl;
            request_->parseSize_ = cur_;
            finalState_ = Success_;
            // request_->debug();
            break;
          } else {
            LOG(INFO) << "Incomplete Request Body, continue Waiting..."
                      << adl::endl;
            finalState_ = Incomplete_;
            /* 不完整就退出本轮解析  */
            break;
          }
        }
      } else {
        LOG(DEBUG) << "Something we can't understand now" << adl::endl;
        finalState_ = Failed_;
        break;
      }
    }
  }
finished:
  return finalState_;
}

} // namespace http
} // namespace adl