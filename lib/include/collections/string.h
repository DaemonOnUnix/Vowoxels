#ifndef BOOSTED_STRING_H
#define BOOSTED_STRING_H

#include <stddef.h>
#include "metaprog/utils.h"
#include "collection_generic.h"

typedef char* pogstr;

typedef struct {
    struct collection_header hdr;
    char data[];
} PACKED _string_t_comp;

typedef _string_t_comp* _string_comp;

#define STR_CAP(x) ((x)->capacity)
#define STR_LEN(x) ((x)->len)

pogstr _string(char* str, size_t optional_size);

Maybe(pogstr) _string_from_fd(int fd, size_t capacity);
pogstr _str_cat(pogstr a, pogstr b);
void _pog_free_str(pogstr* x);

#define smartstr __attribute__((cleanup(_pog_free_str)))

#define string1(x) _string(x, 0) 
#define string2(x, y) _string(x, y)
#define selector2(a, f2, f1, ...) f1
#define string(...) ( selector2(__VA_ARGS__, string2, string1) (__VA_ARGS__) )

#define str_pog_len(x) (GET_HEADER(x)->len)

#endif