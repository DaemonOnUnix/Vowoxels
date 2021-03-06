#ifndef HELPERS_MACROS_H
#define HELPERS_MACROS_H

#include "log/log.h"

#define PACKED __attribute__((packed))
#define ONCE(...) {static char once = 0; if(once) return __VA_ARGS__; once = 1;}
#define UNUSED(...) (void)(__VA_ARGS__)

//
// Bitmanip section
//
#define MASK(n) ((1ull << (n)) -1)
#define MASK_SHIFT(n, shift) (MASK(n) << (shift))
#define SHIFTR(x, n, shift) (((x) >> (shift)) & MASK(n))
#define SHIFTL(x, n, shift) (((x) & MASK(n)) << (shift))


//
// Maybe section
//
#define sizeof_maybe(T) (sizeof(struct PACKED { char some; T val; }))
#define Maybe(T) volatile char*
#define Some(T, V) ({\
  static volatile char v[sizeof_maybe(T)];\
  *(volatile T*)(v + 1) = V;\
  v[0] = 1;\
  v;\
})
#define None(T) ({\
  static volatile char v[sizeof_maybe(T)];\
  v[0] = 0;\
  v;\
})
#define MAYBE_IF(COND, T, V) if(COND) {return Some(T, V);} return None(T);
#define GET_STATUS(V) ((V)[0])
#define GET_VALUE(T, V) (*((T*)((V + 1))))
#define UNWRAP(T, V) ({Maybe(T) ___val = V; if(!GET_STATUS(___val)) PANIC("Unwrap failed.");  GET_VALUE(T, ___val);})
// #define TRY(EXP, T, ON_ERROR) {Maybe(T) ___temp = EXP; if(!GET_STATUS(___temp)) return (ON_ERROR);  }

#endif