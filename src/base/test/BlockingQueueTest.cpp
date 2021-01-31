#include "../src/BlockingQueue.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
void workAndNoPlay(adl::BlockingQueue<int> &bq) {
  while (1) {
    int a = bq.take();
    printf("%d\n", a);
  }
}
int main(int argc, char const *argv[]) {
  std::vector<std::unique_ptr<std::thread>> vt;
  adl::BlockingQueue<int> bq;
  for (size_t i = 0; i < 7; i++) {
    vt.push_back(std::make_unique<std::thread>(workAndNoPlay, std::ref(bq)));
  }
  while (1) {
    bq.put(rand());
  }
  return 0;
}
