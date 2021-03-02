#include "logStream.h"
#include <cmath>
int main(int argc, char const *argv[]) {
  /* code */
  adl::logStream stream;
  int t = 10000;
  const char *s = NULL;
  void *p = NULL;
  int *pp = &t;
  long long ll = 232354562143;
  adl::adlBuffer<4000> bf;
  bf.append("asad", 4);
  stream << 1.0 << 2.12345432 << ll << std::string("pow:") << pow(2, 35)
         << "helo world\n"
         << p << bf << pp;
  auto &b = stream.getBuf();
  b.debugString();
  return 0;
}
