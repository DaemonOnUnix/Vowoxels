#include <stdio.h>
#include "log/log.h"

#include <stdlib.h>
#include "collections/vector.h"

int main(){
    LOG_INFO("Beginning of the program.");
    vector_t(int) plouf = from_initializer(NULL, 10, sizeof(int), 0);
    plouf[9] = 5;
    /*serverentry( (serverdata){
        .port = 8080,
        .max_connections = 10,
        .max_len_buffer = 40,
        .on_connect = connection_handler
    } );*/
    return 0;
}