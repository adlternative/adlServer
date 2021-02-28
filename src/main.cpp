#include "log/Logging.h"
#include "log/asyncLogging.h"
#include "net/EventLoop.h"
#include "net/tcpConnection.h"
#include "net/tcpServer.h"
#include "util.h"

#include <iostream>

int main(int argc, char const *argv[]) {
  adl::asyncLogging *g_asyncLog =
      new adl::asyncLogging("test", 3, 1024 * 1024, 3);

  adl::Logger::setglobalLevel(adl::Logger::TRACE);
  // adl::Logger::setglobalOutFunc(asyncOutput);
  // adl::Logger::setglobalFlushFunc(asyncFlash);
  g_asyncLog->start();

  std::shared_ptr<adl::EventLoop> mainLoop =
      std::make_shared<adl::EventLoop>(true);

  adl::InetAddress addr("127.0.0.1", 9017);
  adl::TcpServer server(mainLoop, addr);
  server.start();
  mainLoop->loop();

  return 0;
}
