#include "../src/Singleton.hpp"
#include <cassert>
#include <iostream>
#include <thread>
using namespace std;
void test_muduo_singleton() {
  using namespace adl::learn_from_muduo;
  auto &a = Singleton<int>::instance();
  a = 1;
  auto t = thread([]() {
    auto &c = Singleton<int>::instance();
    assert(c == 1);
  });
  t.join();
  auto &b = Singleton<int>::instance();
  assert(a == b);
}
void test_use_memory_barrier_sigleton() {
  using namespace adl::use_memory_barrier;
  auto a = Singleton::getInstance();
  // *a = 1;
  // auto t = thread([]() {
  //   auto c = Singleton::getInstance();
  //   assert(*c == 1);
  // });
  // t.join();
  // auto b = Singleton::getInstance();
  // assert(*a == *b);
}

void test_learn_from_starkoverflow() {
  using namespace adl::learn_from_starkoverflow;
  lazy<int> x([] { return new int(1); });
  std::cout << x.get() << std::endl;
}
void test_learn_from_csdn() {
  using namespace adl::learn_from_csdn;
  auto a = Singleton<int>::getInstance();
  *a = 1;
  auto t = thread([]() {
    auto c = Singleton<int>::getInstance();
    assert(*c == 1);
  });
  t.join();
  auto b = Singleton<int>::getInstance();
  assert(a == b);
}
int main(int argc, char const *argv[]) {
  test_learn_from_csdn();
  return 0;
}
