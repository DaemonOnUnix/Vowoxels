#include "voxelengine/data.h"
#include "voxelengine/chunk.h"

#define VERTEX_BUFFER_E(_x, _y, _z, _uvx, _uvy, _nx, _ny, _nz) chunk->vertex_buffer[vertex_count].x = _x;\
    chunk->vertex_buffer[vertex_count].y = _y;\
    chunk->vertex_buffer[vertex_count].z = _z;\
    chunk->vertex_buffer[vertex_count].uv_x = (_uvx);\
    chunk->vertex_buffer[vertex_count].uv_y = (_uvy);\
    chunk->vertex_buffer[vertex_count].normal_x = (_nx);\
    chunk->vertex_buffer[vertex_count].normal_y = (_ny);\
    chunk->vertex_buffer[vertex_count].normal_z = (_nz);\
    vertex_count++;

void updateChunkVertex(Chunk* chunk){
    if(!chunk)
        return;
    EngineData* data = getEngineData();
    struct Atlas* atlas = data->atlas->next;
    size_t vertex_count = 0;
    size_t triangles_count = 0;
    Chunk* neighbour[6];
    neighbour[0] = getChunk(chunk->chunk_x-1, chunk->chunk_y, chunk->chunk_z);
    neighbour[1] = getChunk(chunk->chunk_x+1, chunk->chunk_y, chunk->chunk_z);
    neighbour[2] = getChunk(chunk->chunk_x, chunk->chunk_y-1, chunk->chunk_z);
    neighbour[3] = getChunk(chunk->chunk_x, chunk->chunk_y+1, chunk->chunk_z);
    neighbour[4] = getChunk(chunk->chunk_x, chunk->chunk_y, chunk->chunk_z-1);
    neighbour[5] = getChunk(chunk->chunk_x, chunk->chunk_y, chunk->chunk_z+1);
    if (chunk->vertex_buffer){
        free(chunk->vertex_buffer);
        chunk->vertex_buffer = NULL;
    }
    if (chunk->triangles_buffer){
        free(chunk->triangles_buffer);
        chunk->triangles_buffer = NULL;
    }
    
    for(size_t x = 0; x < CHUNK_DIMENSION; x++){
        for(size_t y = 0; y < CHUNK_DIMENSION; y++){
            for(size_t z = 0; z < CHUNK_DIMENSION; z++){
                uint32_t voxel_id = chunk->voxel_list[INDEX_TO_CHUNK(x, y, z)];

                if(!voxel_id){
                    continue;
                }

                float voxelXPerc = 1.0f / (float)atlas->tilex;
                float voxelYPerc = 1.0f / (float)atlas->tiley;
                float offsetXPerc = 1.0f / (float)(atlas->tilex-atlas->offset_x);
                float offsetYPerc= 1.0f / (float)(atlas->tiley-atlas->offset_y);

                // Check vertex need
                if((x != 0 && !chunk->voxel_list[INDEX_TO_CHUNK(x-1, y, z)]) ||
                (x == 0 && (!neighbour[0] || !neighbour[0]->voxel_list[INDEX_TO_CHUNK(CHUNK_DIMENSION-1, y, z)]))){
                    float xuv = voxelXPerc*atlas->tile_info[voxel_id].tilex[3];
                    float yuv = voxelYPerc*atlas->tile_info[voxel_id].tiley[3];
                    chunk->vertex_buffer = realloc(chunk->vertex_buffer, sizeof(struct Vertex)*(vertex_count+4));
                    chunk->triangles_buffer = realloc(chunk->triangles_buffer, sizeof(struct Vertex)*(triangles_count+6));
                    VERTEX_BUFFER_E(x, y, z+1, xuv+offsetXPerc, yuv+offsetYPerc, -1, 0, 0);
                    VERTEX_BUFFER_E(x, y, z, xuv, yuv+offsetYPerc, -1, 0, 0);
                    VERTEX_BUFFER_E(x, y+1, z+1, xuv+offsetXPerc, yuv, -1, 0, 0);
                    VERTEX_BUFFER_E(x, y+1, z, xuv, yuv, -1, 0, 0);

                    // (0,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 2 - 4;
                    // (0,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (0,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - 4;
                    // (0,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - 4;
                    // (0,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (0,0,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 1 - 4;
                }
                if((x < CHUNK_DIMENSION-1 && !chunk->voxel_list[INDEX_TO_CHUNK(x+1, y, z)]) ||
                (x == CHUNK_DIMENSION-1 && (!neighbour[1] || !neighbour[1]->voxel_list[INDEX_TO_CHUNK(0, y, z)]))) {
                    float xuv = voxelXPerc*atlas->tile_info[voxel_id].tilex[2];
                    float yuv = voxelYPerc*atlas->tile_info[voxel_id].tiley[2];
                    chunk->vertex_buffer = realloc(chunk->vertex_buffer, sizeof(struct Vertex)*(vertex_count+4));
                    chunk->triangles_buffer = realloc(chunk->triangles_buffer, sizeof(struct Vertex)*(triangles_count+6));
                    VERTEX_BUFFER_E(x+1, y+1, z+1, xuv, yuv, 1, 0, 0);
                    VERTEX_BUFFER_E(x+1, y, z+1, xuv, yuv+offsetYPerc, 1, 0, 0);
                    VERTEX_BUFFER_E(x+1, y, z, xuv+offsetXPerc, yuv+offsetYPerc, 1, 0, 0);
                    VERTEX_BUFFER_E(x+1, y+1, z, xuv+offsetXPerc, yuv, 1, 0, 0);
                    
                    // (1,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (1,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 0 - 4;
                    // (1,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 2 - 4;
                    // (1,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 2 - 4;
                    // (1,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 0 - 4;
                    // (1,0,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 1 - 4;
                }
                if((y != 0 && !chunk->voxel_list[INDEX_TO_CHUNK(x, y-1, z)]) ||
                (y == 0 && (!neighbour[2] || !neighbour[2]->voxel_list[INDEX_TO_CHUNK(x, CHUNK_DIMENSION-1, z)]))){
                    float xuv = voxelXPerc*atlas->tile_info[voxel_id].tilex[1];
                    float yuv = voxelYPerc*atlas->tile_info[voxel_id].tiley[1];
                    chunk->vertex_buffer = realloc(chunk->vertex_buffer, sizeof(struct Vertex)*(vertex_count+4));
                    chunk->triangles_buffer = realloc(chunk->triangles_buffer, sizeof(struct Vertex)*(triangles_count+6));
                    VERTEX_BUFFER_E(x, y, z+1, xuv+offsetXPerc, yuv, 0, -1, 0);
                    VERTEX_BUFFER_E(x+1, y, z+1, xuv+offsetXPerc, yuv+offsetYPerc, 0, -1, 0);
                    VERTEX_BUFFER_E(x, y, z, xuv, yuv, 0, -1, 0);
                    VERTEX_BUFFER_E(x+1, y, z, xuv, yuv+offsetYPerc, 0, -1, 0);
                    chunk->triangles_buffer[triangles_count++] = vertex_count-4+3;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-4+1;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-4+2;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-4+2;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-4+1;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-4;
                }
                if((y < CHUNK_DIMENSION-1 && !chunk->voxel_list[INDEX_TO_CHUNK(x, y+1, z)])
                || (y == CHUNK_DIMENSION-1 && (!neighbour[3] || !neighbour[3]->voxel_list[INDEX_TO_CHUNK(x, 0, z)]))){
                    float xuv = voxelXPerc*atlas->tile_info[voxel_id].tilex[0];
                    float yuv = voxelYPerc*atlas->tile_info[voxel_id].tiley[0];
                    chunk->vertex_buffer = realloc(chunk->vertex_buffer, sizeof(struct Vertex)*(vertex_count+4));
                    chunk->triangles_buffer = realloc(chunk->triangles_buffer, sizeof(struct Vertex)*(triangles_count+6));
                    VERTEX_BUFFER_E(x+1, y+1, z+1, xuv+offsetXPerc, yuv+offsetYPerc, 0, 1, 0);
                    VERTEX_BUFFER_E(x, y+1, z+1, xuv, yuv+offsetYPerc, 0, 1, 0);
                    VERTEX_BUFFER_E(x, y+1, z, xuv, yuv, 0, 1, 0);
                    VERTEX_BUFFER_E(x+1, y+1, z, xuv+offsetXPerc, yuv, 0, 1, 0);
                    // (0,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 1 - 4;
                    // (1,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (1,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 2 - 4;
                    // (1,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (0,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 1 - 4;
                    // (0,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 0 - 4;
                }
                if((z != 0 && !chunk->voxel_list[INDEX_TO_CHUNK(x, y, z-1)]) ||
                (z == 0 && (!neighbour[4] || !neighbour[4]->voxel_list[INDEX_TO_CHUNK(x, y, CHUNK_DIMENSION-1)]))){
                    float xuv = voxelXPerc*atlas->tile_info[voxel_id].tilex[4];
                    float yuv = voxelYPerc*atlas->tile_info[voxel_id].tiley[4];
                    chunk->vertex_buffer = realloc(chunk->vertex_buffer, sizeof(struct Vertex)*(vertex_count+4));
                    chunk->triangles_buffer = realloc(chunk->triangles_buffer, sizeof(struct Vertex)*(triangles_count+6));
                    VERTEX_BUFFER_E(x, y, z, xuv+offsetXPerc, yuv+offsetYPerc, 0, 0, -1);
                    VERTEX_BUFFER_E(x+1, y, z, xuv, yuv+offsetYPerc, 0, 0, -1);
                    VERTEX_BUFFER_E(x, y+1, z, xuv+offsetXPerc, yuv, 0, 0, -1);
                    VERTEX_BUFFER_E(x+1, y+1, z, xuv, yuv, 0, 0, -1);
                    // (0,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 2 - 4;
                    // (1,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (0,0,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - 4;
                    // (0,0,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - 4;
                    // (1,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (1,0,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 1 - 4;
                }
                if((z < CHUNK_DIMENSION-1 && !chunk->voxel_list[INDEX_TO_CHUNK(x, y, z+1)]) ||
                (z == CHUNK_DIMENSION-1 && (!neighbour[5] || !neighbour[5]->voxel_list[INDEX_TO_CHUNK(x, y, 0)]))){
                    float xuv = voxelXPerc*atlas->tile_info[voxel_id].tilex[5];
                    float yuv = voxelYPerc*atlas->tile_info[voxel_id].tiley[5];
                    chunk->vertex_buffer = realloc(chunk->vertex_buffer, sizeof(struct Vertex)*(vertex_count+4));
                    chunk->triangles_buffer = realloc(chunk->triangles_buffer, sizeof(struct Vertex)*(triangles_count+6));
                    VERTEX_BUFFER_E(x, y, z+1, xuv, yuv+offsetYPerc, 0, 0, 1);
                    VERTEX_BUFFER_E(x+1, y, z+1, xuv+offsetXPerc, yuv+offsetYPerc, 0, 0, 1);
                    VERTEX_BUFFER_E(x, y+1, z+1, xuv, yuv, 0, 0, 1);
                    VERTEX_BUFFER_E(x+1, y+1, z+1, xuv+offsetXPerc, yuv, 0, 0, 1);
                    // (0,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 2 - 4;
                    // (0,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - 4;
                    // (1,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (1,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 3 - 4;
                    // (0,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - 4;
                    // (1,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count + 1 - 4;
                }
            }
        }
    }
    chunk->vertex_count = vertex_count;
    chunk->triangles_count = triangles_count;
    if(!vertex_count){
        chunk->is_air = true;
    }
}