#ifndef COLLECTION_GENERIC_H
#define COLLECTION_GENERIC_H

#include <stddef.h>

struct collection_header {
    size_t len;
    size_t capacity;
};

#define GET_HEADER(x) ((struct collection_header*)((unsigned long long)x - sizeof(struct collection_header)))
#define FREE_COLLECTION(x) (free(GET_HEADER(x)))

#endif