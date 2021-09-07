#include "collections/string.h"
#include <stdlib.h>
#include <unistd.h>

static size_t my_strlen(char* str){
    size_t r = 0;
    while((r++, *(str++)));
    return r-1;
}

char* my_strcat(char* dest, char* b, size_t location_bet){
    char* a = dest + location_bet;
    if(*a == '\0'){
        while(*b){
            *(a) = *(b);
            a++, b++;
        }
        *a = 0;
    } else {
        location_bet = my_strlen(dest);
        my_strcat(dest, b, location_bet);
    }
    return dest;
}

pogstr _string(char* str, size_t optional_size){
    if(!optional_size)
        optional_size = my_strlen(str) + 1;
    _string_comp to_return = malloc(sizeof(struct collection_header) + optional_size);
    to_return->hdr.len = my_strlen(str);
    to_return->hdr.capacity = optional_size;
    size_t i = 0;
    while(str[i])
        to_return->data[i] = str[i], i++;
    to_return->data[to_return->hdr.capacity - 1] = 0;
    return (pogstr)(to_return->data);
}

Maybe(pogstr) _string_from_fd(int fd, size_t capacity){
    _string_comp buffer = (_string_comp)(GET_HEADER( _string("", capacity) ));
    int e = read(fd, buffer->data, capacity-1);
    buffer->data[e] = 0;
    if(e)
        LOG_OK("Receiving message of length %d, message : %s", e, buffer->data);
    MAYBE_IF(e != -1, pogstr, buffer->data);
}

pogstr _str_cat(pogstr a, pogstr b){
    _string_comp ha = (_string_comp)GET_HEADER(a);
    _string_comp hb = (_string_comp)GET_HEADER(b);
    // LOG_INFO("HA := %p", ha);
    // LOG_INFO("ha.capacity %lu, hb.capacity %lu", ha->hdr.capacity, hb->hdr.capacity);
    // LOG_INFO("Reallocating chunk of size %lu", ha->hdr.capacity + hb->hdr.capacity -1 + sizeof(struct collection_header));
    ha = realloc(ha, ha->hdr.capacity + hb->hdr.capacity -1 + sizeof(struct collection_header));
    pogstr to_return = ha->data;
    ha->hdr.capacity = ha->hdr.capacity + hb->hdr.capacity;
    my_strcat(to_return, b, 0);//ha->hdr.len);
    ha->hdr.len += hb->hdr.len;
    return to_return;
}

void _pog_free_str(pogstr* x){
    free(GET_HEADER((*x)));
}