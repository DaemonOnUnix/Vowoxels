#ifndef LINEAR_ALGEBRA_PERLIN_H
#define LINEAR_ALGEBRA_PERLIN_H

#include <math.h>

// Function to linearly interpolate between a0 and a1
// Weight w should be in the range [0.0, 1.0]
static inline float lerp(float a0, float a1, float w) {
    return (1.0 - w)*a0 + w*a1;
}

#define IYMAX 100
#define IXMAX 100

static inline float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float dotGridGradient(int ix, int iy, float x, float y);
float perlin(float x, float y);
#endif