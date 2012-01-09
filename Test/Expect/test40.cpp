#include <stdlib.h>
#include <propeller.h>
#include "test40.h"

int32_t test40::tx(int32_t character)
{
  int32_t result = 0;
  _OUTA = character;
  return result;
}

int32_t test40::dec(int32_t value)
{
  int32_t result = 0;
  int32_t	i, x;
  x = -(value == -2147483648);
  if (value < 0) {
    value = (abs((value + x)));
    tx('-');
  }
  i = 1000000000;
  for (int32_t _tmp__0000 = 10; _tmp__0000 != 0; _tmp__0000 += -1) {
    if (value >= i) {
      tx((((value / i) + '0') + (x * -(i == 1))));
      value = (value % i);
      __extension__({ int32_t _tmp_ = result; result = -1; _tmp_; });
    } else {
      if ((result) || (i == 1)) {
        tx('0');
      }
    }
    i = (i / 10);
  }
  return result;
}

