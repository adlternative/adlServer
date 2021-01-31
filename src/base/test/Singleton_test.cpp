#include "../Singleton.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
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

void test_thread_local_singleton() {
  std::vector<std::thread> v;
  for (size_t i = 0; i < 2; i++) {
    v.emplace_back([]() {
      int &a = adl::learn_from_muduo::ThreadLocal_Singleton<int>::instance();
      a += 1;
      printf("address:%p,value:%d\n", &a,
             adl::learn_from_muduo::ThreadLocal_Singleton<int>::instance());
      while (1)
        ;
    });
  }
  while (1)
    ;
}
void test_sof_static_singleton() {
  shared_ptr<int> a =
      std::move(adl::learn_from_starkoverflow::Singleton<int>::instance());
  *a = 1;
  auto b = adl::learn_from_starkoverflow::Singleton<int>::instance();
  printf("%d\n", *b);
}
int main(int argc, char const *argv[]) {
  // test_learn_from_csdn();
  // test_thread_local_singleton();
  test_sof_static_singleton();
  return 0;
}
