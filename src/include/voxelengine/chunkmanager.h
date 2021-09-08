#ifndef VOXELENGINE_CHUNK_MANAGER_H
#define VOXELENGINE_CHUNK_MANAGER_H

#include "linear_algebra/vec3.h"
#include "chunk.h"
#include "pthread/helper.h"
#include <pthread.h>
#include <stdatomic.h>

#define VIEW_DIST 5

typedef struct {
    int32_t actual_chunk_x;
    int32_t actual_chunk_y;
    int32_t actual_chunk_z;

    atomic_bool need_update;
    struct chunk_list* chunks;
    pthread_t working_th;
} Chunk_manager;

struct chunk_list {
    int32_t chunk_x;
    int32_t chunk_y;
    int32_t chunk_z;
    Chunk* chunk;
    struct chunk_list* next;
};

void updateChunks(Vec3 camera_pos);

void* thread_loading_chunks(void *args);

#endif