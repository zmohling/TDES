#include "base/factorial.h"

// sup
int Factorial(int i) {
  if (i == 1 || i == 0)
    return 1;
  else
    return i * Factorial(i - 1);
}
