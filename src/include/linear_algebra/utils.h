#ifndef UTILS_H
#define UTILS_H
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define float_max (float)((uint64_t)-1)


#define la_min(a, b) (((a) < (b)) ? (a) : (b))
#define la_max(a, b) (((a) > (b)) ? (a) : (b))
#define la_abs(a) (((a) < 0) ? -(a) : (a))
#define la_floor(a) ((a == (long)a || a >= 0) ? (float)((long)a) : (float)((long)a - 1))

#define la_clamp(a, from, to) la_max(la_min(a, to), from)

static inline uint32_t fast_rand(void)
{
    static uint32_t shuffle_table[4] = {
        2419050644,
        343247216,
    };
    uint32_t s1 = shuffle_table[0];
    uint32_t s0 = shuffle_table[1];
    uint32_t result = s0 + s1;
    shuffle_table[0] = s0;
    s1 ^= s1 << 11;
    shuffle_table[1] = s1 ^ s0 ^ (s1 >> 9) ^ (s0 >> 5);
    return (uint32_t)result;
}

#define la_copysign(x, y) (__builtin_copysign(x, y))

#define random_float() \
    (float)((float)fast_rand() / (float)4294967296)
/* you may want to implement here the fastest implementation */

static inline float mapfloat(float value, float vmin, float vmax, float nmin, float nmax){
    return (value - vmin) * (nmax - nmin) / (vmax - vmin) + nmin;
}

#ifndef NOT_USE_INTRINSIC

#    include <immintrin.h>
#    include <x86intrin.h>

static inline float fast_sqrt(float number)
{
    float a;
    float b = number;
    __m128 in = _mm_load_ss(&b);
    _mm_store_ss(&a, _mm_sqrt_ss(in));
    return a;
}

static inline float Q_rsqrt(float number)
{
    float a;
    float b = number;
    __m128 in = _mm_load_ss(&b);
    _mm_store_ss(&a, _mm_rsqrt_ss(in));
    return a;
}

#else

static inline float fast_sqrt(float number)
{
    return sqrt(number);
}

static inline float Q_rsqrt(float number)
{
    return 1 / sqrt(number);
}

#endif

static inline float fast_atan2(float y, float x)
{
    float abs_y = la_abs(y) + 0.0000000001; // avoid 0 (this may be dumb)
    float r = (x - la_copysign(abs_y, x)) / (abs_y + la_abs(x));
    float angle = (M_PI_2)-la_copysign(M_PI_4, x);

    angle += (0.1963f * r * r - 0.9817f) * r;
    return la_copysign(angle, y);
}

static inline float fast_acos(float x)
{
    return (-0.69813170079773212 * x * x - 0.87266462599716477) * x + 1.5707963267948966;
}

static inline float fast_sin(float x)
{
    x *= M_1_PI * 0.5;
    x -= 0.25 + la_floor(x + 0.25);
    x *= 16 * (la_abs(x) - (0.5));
    return x;
}

static inline float fast_cos(float x)
{
    return fast_sin(M_PI_2 + x);
}

static inline float to_degrees(float radians) {
    return radians * (180.0f / M_PI);
}

static inline float to_radians(float degrees) {
    return degrees * (M_PI / 180.0f);
}

#endif