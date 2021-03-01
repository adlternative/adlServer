#include "log/Logging.h"
#include "log/asyncLogging.h"
#include "net/EventLoop.h"
#include "net/tcpConnection.h"
#include "net/tcpServer.h"
#include "util.h"

#include <iostream>
namespace adl {
void reflectionMessageCallback(const TcpConnectionPtr &conn, netBuffer *buf) {
  const char *content = buf->peek();
  int len = buf->readable();
  LOG(INFO) << "before reflection " << adl::endl;
  conn->send(content, len);
  buf->retrieve(len);
  LOG(INFO) << "after reflection " << adl::endl;
  // conn->debugBuffer();
}
/* 默认的连接回调 */
void reflectionConnectionCallback(const TcpConnectionPtr &conn) {
  LOG(INFO) << "connection setup..." << adl::endl;
}
} // namespace adl

int main(int argc, char const *argv[]) {
  adl::asyncLogging *g_asyncLog =
      new adl::asyncLogging("adlTcpServer", 3, 1024 * 1024, 3);

  adl::Logger::setglobalLevel(adl::Logger::TRACE);
  // adl::Logger::setglobalOutFunc(asyncOutput);
  // adl::Logger::setglobalFlushFunc(asyncFlash);
  g_asyncLog->start();

  std::shared_ptr<adl::EventLoop> mainLoop =
      std::make_shared<adl::EventLoop>(true);

  adl::InetAddress addr("127.0.0.1", 9017);
  std::shared_ptr<adl::TcpServer> server =
      std::make_shared<adl::TcpServer>(mainLoop, addr);
  server->setMessageCallback(adl::reflectionMessageCallback);
  server->start();
  mainLoop->loop();

  return 0;
}
