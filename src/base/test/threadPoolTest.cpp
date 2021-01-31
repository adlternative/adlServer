#include "../src/threadPool.h"
#include <string>
void func1() { printf("func1\n"); }
class Func {
public:
  int func() {
    printf("Func::func\n");
    return 1;
  }
};
int main(int argc, char const *argv[]) {
  adl::threadPool pool;
  for (size_t i = 0; i < 10; i++) {
    pool.addTask(func1);
  }
  for (size_t i = 0; i < 10; i++) {
    pool.addTask(
        [](int i, const std::string &s) { printf("%d-%s\n", i, s.c_str()); },
        std::thread::hardware_concurrency(), "hello");
  }
  Func f;
  for (size_t i = 0; i < 10; i++) {
    auto ff = pool.addTask(std::bind(&Func::func, &f));
    printf("%d\n", ff.get());
  }
  while (1)
    ;
  return 0;
}
