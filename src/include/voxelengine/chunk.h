#ifndef VOXELENGINE_CHUNK_H
#define VOXELENGINE_CHUNK_H

#include <stdint.h>
#include "linear_algebra/vec3.h"
#include "linear_algebra/mat4.h"
#define CHUNK_DIMENSION 32
#define MAX_HEIGHT 60
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
    Vec3 m_pos;
    Vec3 m_rot;
    Vec3 m_scale;
} Transform;

struct Vertex {
    float x; 
    float y;
    float z;
    float uv_x;
    float uv_y;
    float normal_x;
    float normal_y;
    float normal_z;
};

typedef struct {
    int32_t chunk_x;
    int32_t chunk_y;
    int32_t chunk_z;

    bool is_air;
	unsigned int VAO;
	unsigned int VBO[2];

    int32_t voxel_list[CHUNK_NUM_VOXEL];
    enum voxel_mask voxel_mask[CHUNK_NUM_VOXEL];

    struct Vertex *vertex_buffer;
    int64_t vertex_count;

    unsigned int *triangles_buffer;
    int64_t triangles_count;

    Transform transform;
    Mat4 model;
} Chunk;

#define CHUNK_TO_INDEX(index, prefix) {. prefix##x = (index)%(CHUNK_DIMENSION),\
    . prefix##y = ((index)%((CHUNK_DIMENSION)*(CHUNK_DIMENSION))/(CHUNK_DIMENSION)),\
    . prefix##z = ((index)/((CHUNK_DIMENSION)*(CHUNK_DIMENSION)))}
#define INDEX_TO_CHUNK(x, y, z) ((x) + ((CHUNK_DIMENSION) * (y)) + ((CHUNK_DIMENSION) * (CHUNK_DIMENSION) * (z)))
Chunk* newChunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z);
unsigned char saveChunkToFile(Chunk* chunk, char* dir);
char* create_chunk_filename(char* dir, int32_t chunk_x, int32_t chunk_y, int32_t chunk_z);
Chunk* loadChunkFromFile(char* dir, int32_t chunk_x, int32_t chunk_y, int32_t chunk_z);
void updateChunkVertex(Chunk* chunk);
void updateChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
Chunk* generateChunk(int32_t x, int32_t y, int32_t z);


#endif