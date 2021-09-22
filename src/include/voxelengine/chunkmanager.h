#ifndef VOXELENGINE_CHUNK_MANAGER_H
#define VOXELENGINE_CHUNK_MANAGER_H

#include "linear_algebra/vec3.h"
#include "chunk.h"
#include "pthread/helper.h"
#include <pthread.h>
#include <stdatomic.h>
#include "voxelengine/collision.h"

#define VIEW_DIST 10

typedef struct {
    int64_t actual_chunk_x;
    int64_t actual_chunk_y;
    int64_t actual_chunk_z;

    atomic_bool need_update;
    struct chunk_list* chunks;
    pthread_rwlock_t chunkslock;
    pthread_t working_th;
} Chunk_manager;

struct chunk_list {
    Chunk* chunk;
    struct chunk_list* next;
};

Chunk_manager* initChunkManager();

void updateChunks(Vec3 camera_pos);

void* thread_loading_chunks(void *args);

void insertChunkToChunklist(Chunk *chunk);
Chunk* deleteNextChunklist(struct chunk_list* ch_list_prev);
Chunk* getChunk(int32_t x, int32_t y, int32_t z);

void updateCoord(int* dir, int32_t* x, int32_t* y, int32_t* z, int32_t camx, int32_t camy, int32_t camz);
void place_voxel(int32_t voxel_x, int32_t voxel_y, int32_t voxel_z, uint32_t voxel_id);
void place_voxel_to_hit(RaycastHit hit, uint32_t voxel_id);
#endif