#include <propeller.h>
#include "test37.h"

uint8_t test37::dat[] = {
  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
};
int32_t test37::zero(int32_t n)
{
  int32_t result = 0;
  { int32_t _fill__0000; int32_t *_ptr__0002 = (int32_t *)((int32_t *)&dat[0]); int32_t _val__0001 = 0; for (_fill__0000 = n; _fill__0000 > 0; --_fill__0000) {  *_ptr__0002++ = _val__0001; } };
  return result;
}

