#ifndef POGVEC_H
#define POGVEC_H

#include <stddef.h>
#include "metaprog/utils.h"
#include "collection_generic.h"

#define vector void*
#define vector_t(T) T*

struct complete_vector {
    struct collection_header hdr;
    size_t sizeof_element;
    char data[];
};

vector new_empty_vector(size_t to_allocate, size_t elem_size);
vector from_initializer(void* beginning, size_t length, size_t element_size, size_t allocate_excedent);


#define vector2(T) 


#endif