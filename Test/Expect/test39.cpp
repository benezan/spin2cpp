#include <propeller.h>
#include "test39.h"

char test39::dat[] = {
  0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
};
void test39::Setword(int32_t X, int32_t A)
{
  ((uint16_t *)&dat[0])[X] = A;
}

void test39::Setbyte(int32_t X, int32_t B)
{
  ((char *)&dat[16])[X] = B;
}

