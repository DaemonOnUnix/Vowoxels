#include <stdio.h>
#include "log/log.h"
#include <stdlib.h>
#include "collections/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "voxelengine/chunk.h"

void tests(void* pika){
    (void)pika;
}

void test(){
    Chunk* chunk = calloc(1, sizeof(Chunk));
    saveChunkToFile(chunk, "");
}