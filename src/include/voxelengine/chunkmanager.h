#ifndef VOXELENGINE_CHUNK_MANAGER_H
#define VOXELENGINE_CHUNK_MANAGER_H

#include "linear_algebra/vec3.h"
#include "chunk.h"
#include "pthread/helper.h"
#include <pthread.h>
#include <stdatomic.h>

#define VIEW_DIST 1

typedef struct {
    int64_t actual_chunk_x;
    int64_t actual_chunk_y;
    int64_t actual_chunk_z;

    atomic_bool need_update;
    struct chunk_list* chunks;
    atomic_bool can_free;
    struct chunk_list* chunks_toFree;
    pthread_t working_th;
} Chunk_manager;

struct chunk_list {
    Chunk* chunk;
    struct chunk_list* next;
};

Chunk_manager* initChunkManager();

void updateChunks(Vec3 camera_pos);

void* thread_loading_chunks(void *args);

void toFreeChunk();

#endif