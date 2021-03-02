#include "asyncLogging.h"
#include <string>
using namespace std;
adl::asyncLogging *al = NULL;
int main(int argc, char const *argv[]) {
  al = new adl::asyncLogging("test", 3, 1000, 3);
  al->start();
  while (1) {
    string s(40000, 's');
    al->append(s.c_str(), s.length());
    /* code */
    sleep(1);
  }

  while (1)
    ;
  return 0;
}
