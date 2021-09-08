#ifndef VOXELENGINE_CHUNK_H
#define VOXELENGINE_CHUNK_H

#include <stdint.h>

#define CHUNK_DIMENSION 10
#define CHUNK_NUM_VOXEL (CHUNK_DIMENSION * CHUNK_DIMENSION * CHUNK_DIMENSION)

enum voxel_mask {
    Zm = 1,
    Zp = 2,
    Xm = 4,
    Xp = 8,
    Ym = 16,
    Yp = 32
};

typedef struct {
    int32_t chunk_x;
    int32_t chunk_y;
    int32_t chunk_z;

    int32_t voxel_list[CHUNK_NUM_VOXEL];
    enum voxel_mask voxel_mask[CHUNK_NUM_VOXEL];
} Chunk;

#define CHUNK_TO_INDEX(index, prefix) {. prefix##x = (index)%(CHUNK_DIMENSION),\
    . prefix##y = ((index)%((CHUNK_DIMENSION)*(CHUNK_DIMENSION))/(CHUNK_DIMENSION)),\
    . prefix##z = ((index)/((CHUNK_DIMENSION)*(CHUNK_DIMENSION)))}
#define INDEX_TO_CHUNK(x, y, z) ((x) + ((CHUNK_DIMENSION) * (y)) + ((CHUNK_DIMENSION) * (CHUNK_DIMENSION) * z))

#endif