#include <math.h>

double huge = 1.0e300;

double trunc(double x){
	int32_t i0;
  int32_t i1;
  int32_t j0;
	uint32_t i;

  union dshape u;
  u.value = x;
  i0 = u.bits >> 32;
  i1 = (uint32_t) u.bits;

  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;

  if(j0 < 20){
		if(j0 < 0){
			if(huge + x > 0.0){
				i0 &= 0x80000000U;
				i1 = 0;
			}
		} else {
			i = 0x000fffff >> j0;
			if(((i0 & i) | i1) == 0){
        return x;
      }
			if(huge + x > 0.0){
				i0 &= ~i;
				i1 = 0;
			}
		}
	} else if(j0 > 51){
		if(j0 == 0x400){
      return x + x;
    }
		return x;
	} else {
		i = (uint32_t) 0xffffffff >> (j0 - 20);
		if((i1 & i) == 0){
      return x;
    }
		if(huge + x > 0.0){
      i1 &= ~i;
    }
	}

  union dshape u1;
  u1.bits = ((uint64_t)(i0) << 32) | (uint32_t)(i1);
  x = u1.value;

  return x;
}
