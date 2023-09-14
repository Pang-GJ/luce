#include <string>
#include "luce/codec/serializer.h"
#include "luce/common/logger.hpp"
#include "luce/net/rpc/rpc_client.h"
#include "luce/net/rpc_all.h"

struct Student {
  std::string name;
  int age;

  void serialize(codec::Serializer *serializer) const {
    serializer->serialize(name);
    serializer->serialize(age);
  }

  void deserialize(codec::Serializer *serializer) {
    serializer->deserialize(&name);
    serializer->deserialize(&age);
  }
};

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  net::rpc::BlockingRpcClient client("127.0.0.1", 12345);
  int res = client.Call<int>("add", 2, 3).val();
  LOG_INFO("call add response: {}", res);

  Student stu_res = client.Call<Student>("get_stu", "pgj", 21).val();
  LOG_INFO("call get_stu response: name: {}, age: {}", stu_res.name,
           stu_res.age);
  return 0;
}