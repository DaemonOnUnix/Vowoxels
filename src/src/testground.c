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
    chunk->voxel_list[INDEX_TO_CHUNK(5,5,5)] = 1;

    chunk->voxel_list[INDEX_TO_CHUNK(5,5,5)+1] = 1;

    chunk->voxel_list[INDEX_TO_CHUNK(5,5,5)+2] = 1;
    chunk->voxel_mask[INDEX_TO_CHUNK(5,5,5)+2] = 1;
    saveChunkToFile(chunk, "");

    Chunk* chunkload = loadChunkFromFile("", 0, 0, 0);
    LOG_INFO("chunkload->voxel_list[INDEX_TO_CHUNK(5,5,5)-1] : %d",chunkload->voxel_list[INDEX_TO_CHUNK(5,5,5)-1]);
    LOG_INFO("chunkload->voxel_list[INDEX_TO_CHUNK(5,5,5)] : %d",chunkload->voxel_list[INDEX_TO_CHUNK(5,5,5)]);
    LOG_INFO("chunkload->voxel_list[INDEX_TO_CHUNK(5,5,5)+1] : %d",chunkload->voxel_list[INDEX_TO_CHUNK(5,5,5)+1]);
    LOG_INFO("chunkload->voxel_mask[INDEX_TO_CHUNK(5,5,5)+1] : %d",chunkload->voxel_mask[INDEX_TO_CHUNK(5,5,5)+1]);
    LOG_INFO("chunkload->voxel_list[INDEX_TO_CHUNK(5,5,5)+2] : %d",chunkload->voxel_list[INDEX_TO_CHUNK(5,5,5)+2]);
    LOG_INFO("chunkload->voxel_mask[INDEX_TO_CHUNK(5,5,5)+2] : %d",chunkload->voxel_mask[INDEX_TO_CHUNK(5,5,5)+2]);
}