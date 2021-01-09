#include <math.h>

uint32_t B1 = 715094163;
uint32_t B2 = 696219795;

double P0 =  1.87595182427177009643;
double P1 = -1.88497979543377169875;
double P2 =  1.621429720105354466140;
double P3 = -0.758397934778766047437;
double P4 =  0.145996192886612446982;

double cbrt(double x){
	int32_t hx;
	union dshape u;
	double r = 0.0;
  double s = 0.0;
  double t = 0.0;
  double w;
	uint32_t sign;
	uint32_t high;
  uint32_t low;

  union dshape u2;
  u2.value = x;
  hx = u2.bits >> 32;
  low = (uint32_t) u2.bits;

	sign = hx & 0x80000000;

	hx ^= sign;

	if(hx >= 0x7ff00000){
    return x + x;
  }

	if(hx < 0x00100000){


		if ((hx | low) == 0){
      return x;
    }

    union dshape u3;
    u3.value = t;
    u3.bits &= 0xffffffff;
    u3.bits |= (uint64_t)(0x43500000) << 32;
    t = u3.value;
		t *= x;

    u3.value = t;
    high = u3.bits >> 32;

    u3.bits = ((uint64_t)(sign|((high&0x7fffffff)/3+B2)) << 32) | (uint32_t)(0);
    t = u3.value;

  } else {
    union dshape u4;
    u4.value = t;
    u4.bits = ((uint64_t)(sign|(hx/3+B1)) << 32) | (uint32_t)(0);
    t = u4.value;
  }

  r = (t * t) * (t / x);
	t = t * ((P0 + r * (P1 + r * P2)) + ((r * r) * r) * (P3 + r * P4));

	u.value = t;
	u.bits = (u.bits + 0x80000000) & 0xffffffffc0000000ULL;
	t = u.value;

	s = t * t;
	r = x / s;
	w = t + t;
	r = (r - t) / (w + r);
	t = t + t * r;

	return t;
}
