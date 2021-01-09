#include <math.h>

double ldexp(double x, int n){
  double scale;
  if(n > 1023){
    x *= 0x1p1023;
    n -= 1023;
    if(n > 1023){
      x *= 0x1p1023;
      n -= 1023;
      if(n > 1023){
        x = x * 0x1p1023;
        return x;
      }
    }
  } else if(n < -1022){
    x *= 0x1p-1022;
    n += 1022;
    if(n < -1022){
      x *= 0x1p-1022;
      n += 1022;
      if(n < -1022){
        x = x * 0x1p-1022;
        return x;
      }
    }
  }
  union dshape u;
  u.bits = ((uint64_t) ((uint32_t) (0x3ff + n) << 20) | (uint32_t)(0));
  scale = u.value;
  x = x * scale;
  return x;
}
