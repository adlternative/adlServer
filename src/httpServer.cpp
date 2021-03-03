#include "http/httpParser.h"
#include "http/httpRespond.h"
#include "include/headFile.h"
#include "log/Logging.h"
#include "log/asyncLogging.h"
#include "net/EventLoop.h"
#include "net/tcpConnection.h"
#include "net/tcpServer.h"
#include <sys/stat.h>
namespace adl {

void httpMessageCallback(const TcpConnectionPtr &conn, netBuffer *buf) {
  const char *content = buf->peek();
  int len = buf->readable();
  LOG(INFO) << "before dealing client request" << adl::endl;
  http::httpParser parser(content, len);
  auto finalState = parser.parse(); /* 解析状态 */
  auto request = parser.getRequest();

  LOG(INFO) << "finalState= " << static_cast<int>(finalState) << adl::endl;
  // request->debug(); // conn->send(content, len);
  LOG(INFO) << buf_len(content, len) << adl::endl;
  if (finalState == http::httpParser::Success_) {
    LOG(INFO) << "parse success " << request->parseSize_ << "bytes"
              << adl::endl;
    buf->retrieve(request->parseSize_);

    http::httpRespond respond(conn);
    /* 设置版本号 */
    respond.setVersion(request->line_.getVersion());
    if (request->line_.getMethod() == http::GET) {
      struct stat st;
      string file = request->line_.getUrl();
      file = "./source" + file;
      if (file == "./source/")
        file += "index.html";
      /* 设置状态 */
      int fd = xopen(file.c_str(), O_RDONLY);
      if (fd == -1 || lstat(file.c_str(), &st) || !S_ISREG(st.st_mode)) {
        LOG(ERROR) << "no source file: " << file << adl::endl;
        file = "./source/404.html";
        /* 设置状态 */
        respond.setState(http::httpRespond::_404NotFound);
        fd = xopen(file.c_str(), O_RDONLY);

        if (fd == -1 || lstat(file.c_str(), &st)) {
          LOG(ERROR) << "no 404 file: " << file << adl::endl;
          // close user connection
        }
      }

      /* 添加文件大小 */
      respond.addContentSize(st.st_size);
      /* 链接需要发送的文件 */
      respond.setState(http::httpRespond::_200Ok);
      respond.linkFileFd(fd);

      LOG(INFO) << file << "(" << st.st_size << ")"
                << " will send to user." << adl::endl;

      /* 添加文件类型 */
      respond.addContentType("text/html; charset=utf-8");
      /* 添加长短连接 */
      const auto &kepalive = request->head_.getValue("Connection");
      if (kepalive == "Keep-Alive" || kepalive == "keep-alive")
        respond.addKeepAlive(true);
      else
        respond.addKeepAlive(false);

      /* 发送内容 */
      respond.respond();
    }
  } else if (finalState == http::httpParser::Incomplete_) {
    /* nothing to do  */
  } else {
    buf->retrieveAll();
    /* 发送404资源 */
    /* 关闭连接？ */
  }
  LOG(INFO) << "after dealing client request" << adl::endl;
}
/* 默认的连接回调 */
void httpConnectionCallback(const TcpConnectionPtr &conn) {
  LOG(INFO) << "connection setup..." << adl::endl;
}
} // namespace adl

int main(int argc, char const *argv[]) {
  adl::asyncLogging *g_asyncLog =
      new adl::asyncLogging("httpServer", 3, 1024 * 1024, 3);

  adl::Logger::setglobalLevel(adl::Logger::TRACE);
  // adl::Logger::setglobalOutFunc(asyncOutput);
  // adl::Logger::setglobalFlushFunc(asyncFlash);
  g_asyncLog->start();

  std::shared_ptr<adl::EventLoop> mainLoop =
      std::make_shared<adl::EventLoop>(true);

  adl::InetAddress addr("127.0.0.1", 9017);
  std::shared_ptr<adl::TcpServer> server =
      std::make_shared<adl::TcpServer>(mainLoop, addr);
  server->setMessageCallback(adl::httpMessageCallback);
  server->start();
  mainLoop->loop();

  return 0;
}
