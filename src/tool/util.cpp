#include "util.h"
int starts_with(const char *str, const char *prefix) {
  for (;; str++, prefix++)
    if (!*prefix)
      return 1;
    else if (*str != *prefix)
      return 0;
}
