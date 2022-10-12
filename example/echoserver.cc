#include <luce/TcpServer.h>
#include <luce/Logger.h>
#include <functional>

using namespace luce::net;

class EchoServer {
 public:
  EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
    : loop_(loop), server_(loop_, addr, name) {
    // 注册回调函数
    server_.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
    );

    server_.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
      );

    // 设置合理的loop线程数量
    server_.setThreadNum(3);
  }

  void start() {
    server_.start();
  }

 private:
  // 建立连接或者断开时的回调
  void onConnection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
      LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
    } else {
      LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
    }
  }

  // 可读写事件回调
  void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {
  std::string msg = buf->retrieveAllAsString();
    conn->send(msg);
    // conn->shutdown();
  }

  EventLoop *loop_;
  TcpServer server_;
};

int main (int argc, char *argv[])
{
  EventLoop loop;
  InetAddress addr(8000);
  EchoServer server(&loop, addr, "EchoServer-01");
  server.start();
  loop.loop(); // 启动底层Poller
  
  return 0;
}
