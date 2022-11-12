# 接口设计

## 协程网络库应用层接口
```cpp
void OnRequest(TcpConnectoin *conn, TcpServer &server) {
  char buf[1024]; 
  co_await conn->recv(&buf);
  
  // 对buf进行一些业务逻辑操作
  do_job(&buf);

  co_await conn->send(&buf);
}
```
