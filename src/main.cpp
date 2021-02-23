#include "net/EventLoop.h"
#include "net/tcpConnection.h"
#include "net/tcpServer.h"
int main(int argc, char const *argv[]) {

  std::shared_ptr<adl::EventLoop> mainLoop;
  adl::InetAddress addr("127.0.0.1", 9017);
  adl::TcpServer server(mainLoop, addr);
  server.start();
  mainLoop->loop();

  return 0;
}
