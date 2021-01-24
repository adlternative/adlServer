#include "../src/base/currentThread.h"
#include <iostream>

int main(int argc, char const *argv[]) {
  adl::Thread t1([]() { std::cout << "hello" << std::endl; }, "adl");
  t1.start();
  while (1)
    ;
  return 0;
}
