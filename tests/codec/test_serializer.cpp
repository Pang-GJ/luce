#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "luce/codec/serializer.h"
#include "luce/net/rpc_all.h"
using namespace codec;  // NOLINT

// 类 A
class A {
 public:
  A() = default;
  A(const std::string &name, int age) : name(name), age(age) {}

  void serialize(Serializer *serializer) const {
    serializer->serialize(name);
    serializer->serialize(age);
  }

  void deserialize(Serializer *serializer) {
    serializer->deserialize(&name);
    serializer->deserialize(&age);
  }

  bool operator==(const A &rhs) { return age == rhs.age && name == rhs.name; }

 private:
  std::string name;
  int age;
};

// 类 B
class B {
 public:
  B() = default;
  B(const std::string &address, int salary)
      : address(address), salary(salary) {}

  void serialize(Serializer *serializer) const {
    serializer->serialize(address);
    serializer->serialize(salary);
  }

  void deserialize(Serializer *serializer) {
    serializer->deserialize(&address);
    serializer->deserialize(&salary);
  }

  bool operator==(const B &rhs) {
    return address == rhs.address && salary == rhs.salary;
  }

 private:
  std::string address;
  int salary;
};

// 类 C
class C {
 public:
  C() = default;
  C(bool isStudent, const std::string &school)
      : is_student(isStudent), school(school) {}

  void serialize(Serializer *serializer) const {
    serializer->serialize(is_student);
    serializer->serialize(school);
  }

  void deserialize(Serializer *serializer) {
    serializer->deserialize(&is_student);
    serializer->deserialize(&school);
  }

  bool operator==(const C &rhs) {
    return is_student == rhs.is_student && school == rhs.school;
  }

 private:
  bool is_student;
  std::string school;
};

template <std::size_t I = 0, typename... Ts>
typename std::enable_if<I == sizeof...(Ts), void>::type print_tuple(
    const std::tuple<Ts...> &t) {}

template <std::size_t I = 0, typename... Ts>
    typename std::enable_if <
    I<sizeof...(Ts), void>::type print_tuple(const std::tuple<Ts...> &t) {
  auto container = std::get<I>(t);
  for (auto item : container) {
    std::cout << item << " ";
  }
  std::cout << std::endl;
  print_tuple<I + 1, Ts...>(t);
}

TEST_CASE("basic") {
  Serializer serializer;
  A a("Alice", 25);
  B b("123 Main St", 5000);
  C c(true, "ABC School");

  // 序列化
  serializer.serialize(a);
  serializer.serialize(b);
  serializer.serialize(c);

  // 反序列化
  A restoredA;
  B restoredB;
  C restoredC;

  serializer.deserialize(&restoredA);
  serializer.deserialize(&restoredB);
  serializer.deserialize(&restoredC);

  CHECK(a == restoredA);
  CHECK(b == restoredB);
  CHECK(c == restoredC);
}

TEST_CASE("s to s") {
  Serializer serializer;
  A a("Alice", 25);
  B b("123 Main St", 5000);
  C c(true, "ABC School");

  // 序列化
  serializer.serialize(a);
  serializer.serialize(b);
  serializer.serialize(c);

  // 用另一个 Serialize 反序列化
  Serializer out_serializer(serializer.str());
  A restoredA;
  B restoredB;
  C restoredC;

  out_serializer.deserialize(&restoredA);
  out_serializer.deserialize(&restoredB);
  out_serializer.deserialize(&restoredC);

  CHECK(a == restoredA);
  CHECK(b == restoredB);
  CHECK(c == restoredC);
}

TEST_CASE("container") {
  Serializer serializer;
  std::vector<int> vec = {1, 2, 3, 4, 5};
  std::deque<int> deq = {6, 7, 8, 9, 10};
  std::list<int> l = {11, 12, 13, 14, 15};
  std::forward_list<int> f_l = {16, 17, 18, 19, 20};

  auto in_tuple = std::make_tuple(vec, deq, l, f_l);

  print_tuple(in_tuple);

  serializer.serialize(in_tuple);

  std::tuple<std::vector<int>, std::deque<int>, std::list<int>,
             std::forward_list<int>>
      out_tuple;
  serializer.deserialize(&out_tuple);

  print_tuple(out_tuple);
}