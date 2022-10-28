#pragma once

#include <cstddef>

namespace net {

class Socket;
class AsyncAccept;
class AsyncSend;
class AsyncRecv;

class IOContext {
 public:
  IOContext();

  void Run();

 private:
  friend Socket;
  friend AsyncAccept;
  friend AsyncRecv;
  friend AsyncSend;

  void Attach(Socket *socket);
  void Detach(Socket *socket);

  void WatchRead(Socket *socket);
  void UnWatchRead(Socket *socket);

  void WatchWrite(Socket *socket);
  void UnWatchWrite(Socket *socket);

  inline void UpdateState(Socket *socket, unsigned int new_state);

  // TODO(pgj): update max events
  constexpr static std::size_t max_events = 10;
  const int fd_;
};

}  // namespace net