#include "../adlBuffer.h"
#include <iostream>
#define DEBUG_CLS_RET(cls, var, func)                                          \
  do {                                                                         \
    std::cout << #cls << " " << #var << " " << #func << " :" << var.func       \
              << std::endl;                                                    \
  } while (0)

int main(int argc, char const *argv[]) {
  adl::adlBuffer<4000> buf;
  const char *s = "adl";
  const char *s2 = "fr";
  printf("%p\n", s);
  exit(1);
  while (buf.avail() >= strlen(s) + 1)
    buf.append(s, strlen(s) + 1);
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, begin());
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, size());
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, toString());
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, current());
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, avail());
  if (buf.avail() >= strlen(s2) + 1)
    buf.append(s2, strlen(s2) + 1);
  buf.debugString(true);
  // buf.reset(true);
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, begin());
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, size());
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, toString());
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, current());
  DEBUG_CLS_RET(adl::adlBuffer<4000>, buf, avail());
  return 0;
}
