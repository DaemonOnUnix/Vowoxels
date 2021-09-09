#include "collections/vector.h"
#include <stdlib.h>
static void* zero_out(void* beginning, size_t len){
    char* converted = (char*)beginning;
    while(len-- > 0){
        converted[len] = 0;
    }
    return beginning;
}

static struct complete_vector* NULL_case(size_t length,
    size_t element_size,
    size_t allocate_excedent){
    struct complete_vector* to_return = 
        malloc(sizeof(struct complete_vector) +
            element_size * (length + allocate_excedent)
        );
    (void)zero_out(to_return->data, element_size * (length + allocate_excedent));
    to_return->sizeof_element = element_size;
    to_return->hdr.capacity = element_size * (length + allocate_excedent);
    to_return->hdr.len = element_size * (length + allocate_excedent);
    return to_return;
}

vector from_initializer(void* beginning, 
    size_t length,
    size_t element_size,
    size_t allocate_excedent){
    if(!(unsigned long long)beginning)
        return NULL_case(length, element_size, allocate_excedent)->data;
    struct complete_vector* to_return = 
        malloc(sizeof(struct complete_vector) +
            element_size * (length + allocate_excedent)
        );
    (void)zero_out(to_return->data, element_size * (length + allocate_excedent));
    to_return->sizeof_element = element_size;
    to_return->hdr.capacity = element_size * (length + allocate_excedent);
    to_return->hdr.len = element_size * (length + allocate_excedent);
    for(char* ptr = beginning; length > 0; length--)
        ptr[length-1] = ((char*)beginning)[length-1];
    return to_return->data;
}
