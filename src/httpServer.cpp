#include "./httpServer.h"
#include <sys/stat.h>

namespace adl {
namespace http {
void httpServer::httpMessageCallback(const TcpConnectionPtr &conn,
                                     netBuffer *buf) {

  const char *content = buf->peek();
  int len = buf->readable();
  LOG(INFO) << "before dealing client request" << adl::endl;
  http::httpParser parser(content, len);

  LOG(INFO) << "\nraw messages: " << buf_len(content, len) << adl::endl;
  auto finalState = parser.parse(); /* 解析http请求 */
  LOG(INFO) << "finalState = " << static_cast<int>(finalState) << adl::endl;

  auto request = parser.getRequest(); /* 获得解析出来的请求对象 */

  if (finalState == http::httpParser::Success_) {
    successCallback(conn, request, buf);
  } else if (finalState == http::httpParser::Incomplete_) {
    /* nothing to do  */
  } else {
    buf->retrieveAll();
    /* 关闭连接？ */
    conn->shutdown();
  }
  LOG(INFO) << "after dealing client request" << adl::endl;
}

void httpServer::httpConnectionCallback(const TcpConnectionPtr &conn) {
  LOG(INFO) << "connection setup..." << adl::endl;
}

void httpServer::successCallback(const TcpConnectionPtr &conn,
                                 const std::shared_ptr<httpRequest> &request,
                                 netBuffer *buf) {
  LOG(INFO) << "parse success " << request->parseSize_ << "bytes" << adl::endl;

  struct stat st;
  http::httpRespond respond(conn);
  /* 取走解析的请求的bytes */
  buf->retrieve(request->parseSize_);
  string url = request->line_.getUrl();

  /* 设置版本号 */
  respond.setVersion(request->line_.getVersion());
  /* 设置长短连接 */
  const auto &kepalive = request->head_.getValue("Connection");
  bool kep = kepalive == "Keep-Alive" || kepalive == "keep-alive";
  respond.addKeepAlive(kep);
  if (request->line_.getMethod() == http::GET) {
    /* 解析了需要发送的文件并且设置了状态码 */
    int fd = parseUrl(url, st, respond);
    if (fd < 0) {
      conn->forceClose();
      return;
    }
    /* 添加文件大小 */
    respond.addContentSize(st.st_size);
    /* 链接需要发送的文件 */
    respond.linkFileFd(fd);
    LOG(INFO) << url << "(" << st.st_size << "bytes)"
              << " will send to user." << adl::endl;
    /* 添加消息类型 */
    respond.addContentType("text/html; charset=utf-8");
    /* 发送内容 */
    respond.respond();

  } else if (request->line_.getMethod() == http::POST) {
    fileApp file("./source/" + std::to_string(rand()) + ".txt");
    file.append(request->body_.message_.c_str(),
                request->body_.message_.size());
    /* 设置了状态码 */
    respond.setStatusCode(http::httpRespond::_200Ok);
    /* 添加消息类型 */
    respond.addContentType("text/html; charset=utf-8");
    /* 添加消息体：这里只是一段话 */
    respond.setBody("which your sended have saved as ./source" + url);
    /* 发送回复 */
    respond.respond();
  }
  /* 短链接则关闭 */
  if (!kep)
    conn->forceClose();
}

int httpServer::parseUrl(string &url, struct stat &st,
                         http::httpRespond &respond) {
  url = "./source" + url;
  if (url == "./source/")
    url += "index.html";
  int fd = xopen(url.c_str(), O_RDONLY);
  if (fd == -1 || lstat(url.c_str(), &st) || !S_ISREG(st.st_mode)) {
    LOG(ERROR) << "open " << url << "failed" << adl::endl;
    url = "./source/404.html";
    fd = xopen(url.c_str(), O_RDONLY);
    if (fd == -1 || lstat(url.c_str(), &st)) {
      LOG(ERROR) << "open " << url << "failed" << adl::endl;
      // close user connection
    }
    respond.setStatusCode(http::httpRespond::_404NotFound);
  } else
    respond.setStatusCode(http ::httpRespond::_200Ok);
  return fd;
}

} // namespace http
} // namespace adl

int main(int argc, char const *argv[]) {
  std::unique_ptr<adl::asyncLogging> g_asyncLog =
      std::make_unique<adl::asyncLogging>("httpServer", 3, 1024 * 1024, 3);

  adl::Logger::setglobalLevel(adl::Logger::TRACE);
  g_asyncLog->start();

  adl::InetAddress addr("127.0.0.1", 9017);
  adl::http::httpServer server(addr);
  server.start();
  return 0;
}
