#include "http/httpParser.h"
#include "log/Logging.h"
#include "log/asyncLogging.h"
#include "net/EventLoop.h"
#include "net/tcpConnection.h"
#include "net/tcpServer.h"
#include "tool/usage.h"
#include <iostream>
namespace adl {

void httpMessageCallback(const TcpConnectionPtr &conn, netBuffer *buf) {
  const char *content = buf->peek();
  int len = buf->readable();
  LOG(INFO) << "before dealing" << adl::endl;
  http::httpParser parser(content, len);
  auto finalState = parser.parse(); /* 解析状态 */
  auto request = parser.getRequest();
  LOG(INFO) << "finnalState= " << static_cast<int>(finalState) << adl::endl;
  // request->debug(); // conn->send(content, len);
  LOG(INFO) << buf_len(content, len) << adl::endl;
  buf->retrieve(len);
  LOG(INFO) << "after  dealing" << adl::endl;
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
