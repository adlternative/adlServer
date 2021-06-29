#include <iostream>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
pid_t gettid() {
  /* 获得/系统内唯一的线程pid */
  return static_cast<pid_t>(::syscall(SYS_gettid));
}
void test_tid() {
  std::cout << std::this_thread::get_id() << std::endl;
  printf("%d\n", gettid());
  printf("%ld\n", pthread_self());
}

int main(int argc, char const *argv[]) {
  test_tid();
  exit(0);
  {
    std::thread t([]() {
      while (1)
        printf("123\n");
    });
    t.detach();
  }
  while (1)
    ;
  return 0;
}
