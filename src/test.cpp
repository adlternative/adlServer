#include "util.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
std::string getTime() {
  time_t tt = time(NULL);
  struct tm *stm = localtime(&tt);

  char tmp[32];
  sprintf(tmp, "%04d-%02d-%02d-%02d-%02d-%02d", 1900 + stm->tm_year,
          1 + stm->tm_mon, stm->tm_mday, stm->tm_hour, stm->tm_min,
          stm->tm_sec);

  return tmp;
}

int main(int argc, char const *argv[]) {
  std::cout << getTime() << std::endl;
  return 0;
  errno = EINTR;
}
