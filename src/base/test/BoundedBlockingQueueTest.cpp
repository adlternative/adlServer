#include "../src/BoundedBlockingQueue.h"
#include <atomic>
#include <functional>
#include <iostream>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <thread>
#include <vector>
typedef std::function<void()> Functor;
adl::BoundedBlockingQueue<Functor> bbq(3);
void workThread() {
  while (1) {
    Functor t = bbq.take();
    t();
  }
}
void func() { printf("%d\n", static_cast<pid_t>(::syscall(SYS_gettid))); }
void func2() { printf("%ld\n", std::this_thread::get_id()); }
void func3(int i) {
  printf("rand:%d\n", i);
  sleep(1);
}
int main(int argc, char const *argv[]) {
  int N = 8;
  std::vector<Functor> funcVec;
  funcVec.push_back(func);
  funcVec.push_back(func2);
  std::atomic<int> ii(0);
  funcVec.push_back(std::bind(func3, std::ref(ii)));
  std::vector<std::unique_ptr<std::thread>> tvec;
  for (size_t i = 0; i < N - 1; i++) {
    tvec.push_back(std::make_unique<std::thread>(workThread));
  }
  while (1) {
    ii.store(ii.load() + 1);
    bbq.put(funcVec[rand() % (funcVec.size())]);
  }
  for (auto &&i : tvec) {
    i->join();
  }
  tvec.clear();
  return 0;
}
