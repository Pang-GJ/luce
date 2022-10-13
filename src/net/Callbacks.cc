#include "net/Callbacks.h"
#include "net/Buffer.h"
#include "net/TcpConnection.h"

namespace luce::net {

void defaultConnectionCallback(const TcpConnectionPtr &conn) {
  LOG_INFO("%s -> %s is %s", conn->localAddress().toIpPort().c_str(), conn->peerAddress().toIpPort().c_str(),
           (conn->connected() ? "UP" : "DOWN"));
}

void defaultMessageCallback(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) { buf->retrieveAll(); }

} // namespace luce::net