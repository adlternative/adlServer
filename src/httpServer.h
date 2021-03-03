#ifndef HTTPSERVER_H
#include "http/httpParser.h"
#include "http/httpRespond.h"
#include "include/headFile.h"
#include "log/Logging.h"
#include "log/asyncLogging.h"
#include "net/EventLoop.h"
#include "net/tcpConnection.h"
#include "net/tcpServer.h"

namespace adl {

namespace http {
class httpServer {
public:
  httpServer(const InetAddress &listenAddr)
      : mainLoop_(std::make_shared<adl::EventLoop>(true)),
        tcpServer_(std::make_shared<adl::TcpServer>(mainLoop_, listenAddr)) {
    srand(time(nullptr));
  }

  /* 默认的消息回调 */
  void httpMessageCallback(const TcpConnectionPtr &conn, netBuffer *buf);
  /* 默认的连接回调 */
  void httpConnectionCallback(const TcpConnectionPtr &conn);

  void successCallback(const TcpConnectionPtr &conn,
                       const std::shared_ptr<httpRequest> &request,
                       netBuffer *buf);
  int parseUrl(string &url, struct stat &st, http::httpRespond &respond);
  void start() {
    tcpServer_->setConnectionCallback(
        [this](const TcpConnectionPtr &conn) { httpConnectionCallback(conn); });
    tcpServer_->setMessageCallback(
        [this](const TcpConnectionPtr &conn, netBuffer *buf) {
          httpMessageCallback(conn, buf);
        });
    tcpServer_->start();
    mainLoop_->loop();
  }

private:
  std::shared_ptr<adl::EventLoop> mainLoop_;
  std::shared_ptr<adl::TcpServer> tcpServer_;
};

} // namespace http

}; // namespace adl

#define HTTPSERVER_H
#endif