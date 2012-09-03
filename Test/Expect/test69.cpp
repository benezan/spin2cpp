#include <propeller.h>
#include "test69.h"

#ifdef __GNUC__
#define INLINE__ static inline
#define PostEffect__(X, Y) __extension__({ int32_t tmp__ = (X); (X) = (Y); tmp__; })
#else
#define INLINE__ static
static int32_t tmp__;
#define PostEffect__(X, Y) (tmp__ = (X), (X) = (Y), tmp__)
#endif

int32_t test69::Demo(void)
{
  int32_t result = 0;
  if (!(((INA >> 0) & 0x1))) {
    abort();
  } else {
    if (!(((INA >> 1) & 0x1))) {
      OUTA &= ~(7<<0);
    }
  }
  return result;
}

