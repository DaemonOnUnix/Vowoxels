#ifndef POG_PTR_POGPTR_H
#define POG_PTR_POGPTR_H

#define __POGPTR__CAT3(x, y, z) x##y##z
#define MIAOU(...) __POGPTR__CAT3(__VA_ARGS__)
#define __pogptr(T, count) void MIAOU(___cleaner, count, __LINE__)(T** x){ free(*x); }; __attribute__((cleanup( MIAOU(___cleaner, count, __LINE__) ))) T*
#define pogptr(T) __pogptr(T, __COUNT__)

#endif