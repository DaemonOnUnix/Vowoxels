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

#define FACE_ID_IF(_face, _index) {if (atlas->tile_info[index_x + index_y*atlas->tilex].facemask & (_face)){\
        tilesx_index[(_index)] = index_x;\
        tilesy_index[(_index)] = index_y;\
        found++;\
    }}

void updateChunkVertex(Chunk* chunk){
    EngineData* data = getEngineData();
    struct Atlas* atlas = data->atlas->next;
    size_t vertex_count = 0;
    size_t triangles_count = 0;
    for(size_t x = 0; x < CHUNK_DIMENSION; x++){
        for(size_t y = 0; y < CHUNK_DIMENSION; y++){
            for(size_t z = 0; z < CHUNK_DIMENSION; z++){
                uint32_t voxel_id = chunk->voxel_list[INDEX_TO_CHUNK(x, y, z)];

                // Find tile index with voxel_id
                uint32_t tilesx_index[6] = {0};
                uint32_t tilesy_index[6] = {0};
                char found = 0;
                for (uint32_t index_x = 0; found < 6 && index_x < atlas->tilex; index_x++)
                {
                    for (uint32_t index_y = 0; found < 6 && index_y < atlas->tiley; index_y++)
                    {
                        if (atlas->tile_info[index_x + index_y*atlas->tilex].id == voxel_id)
                        {
                            FACE_ID_IF(FACE_UP, 0u)
                            FACE_ID_IF(FACE_DOWN, 1u)
                            FACE_ID_IF(FACE_NORTH, 2u)
                            FACE_ID_IF(FACE_SOUTH, 3u)
                            FACE_ID_IF(FACE_EAST, 4u)
                            FACE_ID_IF(FACE_WEST, 5u)
                        }
                    }
                    
                }

                if(!voxel_id){
                    continue;
                }

                float voxelXPerc = 1.0f / (float)atlas->tilex;
                float voxelYPerc = 1.0f / (float)atlas->tiley;
                float offsetXPerc = 1.0f / (float)(atlas->tilex-atlas->offset_x);
                float offsetYPerc= 1.0f / (float)(atlas->tiley-atlas->offset_y);

                // Check vertex need
                if((x == 0 || !chunk->voxel_list[INDEX_TO_CHUNK(x-1, y, z)])){
                    float xuv = voxelXPerc*tilesx_index[3];
                    float yuv = voxelYPerc*tilesy_index[3];
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
                if((x == CHUNK_DIMENSION-1 || !chunk->voxel_list[INDEX_TO_CHUNK(x+1, y, z)])) {
                    float xuv = voxelXPerc*tilesx_index[2];
                    float yuv = voxelYPerc*tilesy_index[2];
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
                if((y == 0 || !chunk->voxel_list[INDEX_TO_CHUNK(x, y-1, z)]) ){
                    float xuv = voxelXPerc*tilesx_index[1];
                    float yuv = voxelYPerc*tilesy_index[1];
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
                if((y == CHUNK_DIMENSION-1 || !chunk->voxel_list[INDEX_TO_CHUNK(x, y+1, z)]) ){
                    float xuv = voxelXPerc*tilesx_index[0];
                    float yuv = voxelYPerc*tilesy_index[0];
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
                if((z == 0 || !chunk->voxel_list[INDEX_TO_CHUNK(x, y, z-1)])){
                    float xuv = voxelXPerc*tilesx_index[4];
                    float yuv = voxelYPerc*tilesy_index[4];
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
                if((z == CHUNK_DIMENSION-1 || !chunk->voxel_list[INDEX_TO_CHUNK(x, y, z+1)])){
                    float xuv = voxelXPerc*tilesx_index[5];
                    float yuv = voxelYPerc*tilesy_index[5];
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