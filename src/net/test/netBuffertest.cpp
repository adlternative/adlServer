#include "netBuffer.h"
using namespace adl;
using namespace std;
// int main(int argc, char const *argv[]) {
//   netBuffer buf;
//   char str[4 * 4096];
//   int *i = new int(65);
//   for (int i = 0; i != 4096; i++)
//     buf.append("abc", 4);
//   buf.append(i, 4);
//   memcpy(str, buf.peek(), 4096 * 4);
//   for (int i = 0; i != 4096; i++) {
//     printf("%c ", str[i]);
//   }
//   printf("\n");
//   printf("r:%d\n", buf.readable());
//   printf("w:%d\n", buf.writeable());
//   buf.retrieve(4 * 4096);
//   printf("r:%d\n", buf.readable());
//   printf("w:%d\n", buf.writeable());

//   printf("数字：%d\n", buf.readInt());
//   printf("r:%d\n", buf.readable());
//   printf("w:%d\n", buf.writeable());

//   delete (i);
//   return 0;
// }
