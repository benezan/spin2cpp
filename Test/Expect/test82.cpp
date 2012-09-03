#include <propeller.h>
#include "test82.h"

#ifdef __GNUC__
#define INLINE__ static inline
#define PostEffect__(X, Y) __extension__({ int32_t tmp__ = (X); (X) = (Y); tmp__; })
#else
#define INLINE__ static
static int32_t tmp__;
#define PostEffect__(X, Y) (tmp__ = (X), (X) = (Y), tmp__)
#endif

int32_t test82::Flip(void)
{
  int32_t result = 0;
  X = (~X);
  return result;
}

int32_t test82::Toggle(int32_t Pin)
{
  int32_t result = 0;
  OUTA ^= (1<<Pin);
  return result;
}

