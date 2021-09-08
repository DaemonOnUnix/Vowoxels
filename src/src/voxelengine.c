#include "voxelengine/chunk.h"
#include "collections/string.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>

#define bool unsigned char

void newChunk(Chunk* new){
    UNUSED(new);
}

pogstr create_filename(char* dir, int32_t chunk_x, int32_t chunk_y, int32_t chunk_z){
    // Create filename
    pogstr filename = string(dir);
    LOG_INFO("Filename '%s' length : %ld", filename, str_pog_len(filename));
    char buffer[50] = {0};
    sprintf(buffer, "%u_%u_%u", (uint32_t)chunk_x, (uint32_t)chunk_y, (uint32_t)chunk_z);
    filename = _str_cat(filename, string(buffer));
    LOG_INFO("Filename : %s", filename);
    return filename;
}

bool saveChunkToFile(Chunk* chunk, char* dir){
    
    // Create filename
    smartstr pogstr filename = create_filename(dir, chunk->chunk_x, chunk->chunk_y, chunk->chunk_z);

    int chunk_fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if(!chunk_fd) {
        LOG_PANIC("Error while loading chunk file '%s'", filename);
        return 0;
    }

    // Write Chunk
    for(int32_t voxel_count = 0; voxel_count < CHUNK_NUM_VOXEL;){
        // Search how many same voxel in raw
        int32_t voxel_type = chunk->voxel_list[voxel_count];
        int32_t voxel_mask = chunk->voxel_mask[voxel_count];
        int32_t voxel_num = 1;
        voxel_count++;
        for (; voxel_count < CHUNK_NUM_VOXEL
            && chunk->voxel_list[voxel_count] == voxel_type
            && chunk->voxel_mask[voxel_count] == (enum voxel_mask)voxel_mask;
            voxel_count++, voxel_num++)
        {}

        // Write data
        if(!write(chunk_fd, &voxel_num, sizeof(int32_t))) {
            LOG_PANIC("Error to write voxel_num in chunk file '%s'", filename);
            close(chunk_fd);
            return 0;
        }
        if(!write(chunk_fd, &voxel_type, sizeof(int32_t))) {
            LOG_PANIC("Error to write voxel_type in chunk file '%s'", filename);
            close(chunk_fd);
            return 0;
        }
        if(!write(chunk_fd, &voxel_mask, sizeof(int32_t))) {
            LOG_PANIC("Error to write voxel_mask in chunk file '%s'", filename);
            close(chunk_fd);
            return 0;
        }
    }

    close(chunk_fd);
    return 1;
}

Chunk* loadChunkFromFile(char* dir, int32_t chunk_x, int32_t chunk_y, int32_t chunk_z){

    // Create filename
    smartstr pogstr filename = create_filename(dir, chunk_x, chunk_y, chunk_z);

    // Read Chunk
    int chunk_fd = open(filename, O_RDONLY);
    if(!chunk_fd) {
        LOG_PANIC("Error while loading chunk file '%s'", filename);
        return NULL;
    }
    smartstr pogstr _data = UNWRAP(pogstr, _string_from_fd(chunk_fd, 0));
    int32_t* data = (int32_t*) _data;
    close(chunk_fd);

    // Allocate Chunk
    Chunk* to_return = malloc(sizeof(Chunk));

    // Parse data
    size_t count = 0;
    size_t voxel_count = 0;
    for(size_t i = 0; count < CHUNK_NUM_VOXEL; i+=3) {
        int32_t num = data[i];
        int32_t voxel_type = data[i+1];
        int32_t voxel_mask = data[i+2];
        count += num;

        for(int32_t index = 0; index < num; index++, voxel_count++) {
            to_return->voxel_list[voxel_count] = voxel_type;
            to_return->voxel_mask[voxel_count] = (enum voxel_mask)voxel_mask;
        }

    }

    return to_return;
}

#define VERTEX_BUFFER_E(_x, _y, _z) chunk->vertex_buffer[vertex_count].x = _x;\
    chunk->vertex_buffer[vertex_count].y = _y;\
    chunk->vertex_buffer[vertex_count].z = _z;\
    vertex_count++;

#define SUMMIT_sde ((unsigned char)0b00000001)
#define SUMMIT_nde ((unsigned char)0b00000010)
#define SUMMIT_sdw ((unsigned char)0b00000100)
#define SUMMIT_ndw ((unsigned char)0b00001000)
#define SUMMIT_sue ((unsigned char)0b00010000)
#define SUMMIT_nue ((unsigned char)0b00100000)
#define SUMMIT_suw ((unsigned char)0b01000000)
#define SUMMIT_nuw ((unsigned char)0b10000000)

#define SUMMIT_u ((unsigned char)0b00000001)
#define SUMMIT_d ((unsigned char)0b00000010)
#define SUMMIT_n ((unsigned char)0b00000100)
#define SUMMIT_s ((unsigned char)0b00001000)
#define SUMMIT_e ((unsigned char)0b00010000)
#define SUMMIT_w ((unsigned char)0b00100000)

void updateChunkVertex(Chunk* chunk){
    size_t vertex_count = 0;
    size_t triangles_count = 0;
    for(size_t x = 0; x < CHUNK_DIMENSION; x++){
        for(size_t y = 0; y < CHUNK_DIMENSION; y++){
            for(size_t z = 0; z < CHUNK_DIMENSION; z++){
                if(chunk->voxel_list[INDEX_TO_CHUNK(x, y, z)] != 0){
                    continue;
                }

                char vertex_mask = 0;
                char face_mask = 0;
                char nb_vertex = 0;

                // Check vertex need
                if(x > 0 && !chunk->voxel_list[INDEX_TO_CHUNK(x-1, y, z)]){
                    vertex_mask |= (SUMMIT_sue | SUMMIT_suw | SUMMIT_sde | SUMMIT_sdw);
                    face_mask |= SUMMIT_s;
                }
                if(x < CHUNK_DIMENSION-1 && !chunk->voxel_list[INDEX_TO_CHUNK(x+1, y, z)]){
                    vertex_mask |= (SUMMIT_nue | SUMMIT_nuw | SUMMIT_nde | SUMMIT_ndw);
                    face_mask |= SUMMIT_n;
                }
                if(y > 0 && !chunk->voxel_list[INDEX_TO_CHUNK(x, y-1, z)]){
                    vertex_mask |= (SUMMIT_nde | SUMMIT_nde | SUMMIT_ndw | SUMMIT_ndw);
                    face_mask |= SUMMIT_d;
                }
                if(y < CHUNK_DIMENSION-1 && !chunk->voxel_list[INDEX_TO_CHUNK(x, y+1, z)]){
                    vertex_mask |= (SUMMIT_nue | SUMMIT_sue | SUMMIT_nuw | SUMMIT_suw);
                    face_mask |= SUMMIT_u;
                }
                if(z > 0 && !chunk->voxel_list[INDEX_TO_CHUNK(x, y, z+1)]){
                    vertex_mask |= (SUMMIT_nde | SUMMIT_sde | SUMMIT_nue | SUMMIT_sue);
                    face_mask |= SUMMIT_e;
                }
                if(z < CHUNK_DIMENSION-1 && !chunk->voxel_list[INDEX_TO_CHUNK(x, y, z+1)]){
                    vertex_mask |= (SUMMIT_ndw | SUMMIT_sdw | SUMMIT_nuw | SUMMIT_suw);
                    face_mask |= SUMMIT_w;
                }

                // Add vertex to buffer
                if(vertex_mask & SUMMIT_sde){
                    VERTEX_BUFFER_E(x, y, z+1);
                    nb_vertex++;
                }
                if(vertex_mask & SUMMIT_nde){
                    VERTEX_BUFFER_E(x+1, y, z+1);
                    nb_vertex++;
                }
                if(vertex_mask & SUMMIT_sdw){
                    VERTEX_BUFFER_E(x, y, z);
                    nb_vertex++;
                }
                if(vertex_mask & SUMMIT_ndw){
                    VERTEX_BUFFER_E(x+1, y, z);
                    nb_vertex++;
                }
                if(vertex_mask & SUMMIT_sue){
                    VERTEX_BUFFER_E(x, y+1, z+1);
                    nb_vertex++;
                }
                if(vertex_mask & SUMMIT_suw){
                    VERTEX_BUFFER_E(x, y+1, z);
                    nb_vertex++;
                }
                if(vertex_mask & SUMMIT_nuw){
                    VERTEX_BUFFER_E(x+1, y+1, z);
                    nb_vertex++;
                }
                if(vertex_mask & SUMMIT_nue){
                    VERTEX_BUFFER_E(x+1, y+1, z+1);
                    nb_vertex++;
                }
                
                // Add triangles to buffer
                char offset = 0;
                if(face_mask & SUMMIT_d){
                    chunk->triangles_buffer[triangles_count++] = vertex_count-nb_vertex+3;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-nb_vertex+1;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-nb_vertex+2;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-nb_vertex+2;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-nb_vertex+1;
                    chunk->triangles_buffer[triangles_count++] = vertex_count-nb_vertex;
                }
                if(face_mask & SUMMIT_e){
                    // (0,1,1)
                    offset = ((vertex_mask & SUMMIT_sdw) >> 2) + ((vertex_mask & SUMMIT_ndw) >> 3);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 2 + offset;
                    // (0,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex;
                    // (1,1,1)
                    offset = ((vertex_mask & SUMMIT_sdw) >> 2) + ((vertex_mask & SUMMIT_ndw) >> 3) + ((vertex_mask & SUMMIT_suw) >> 6) + ((vertex_mask & SUMMIT_nuw) >> 7);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 3 + offset;
                    // (1,1,1)
                    offset = ((vertex_mask & SUMMIT_sdw) >> 2) + ((vertex_mask & SUMMIT_ndw) >> 3);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 3 + offset;
                    // (0,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex;
                    // (1,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 1;
                }
                if(face_mask & SUMMIT_w){
                    // (0,1,0)
                    offset = (vertex_mask & SUMMIT_sde) + ((vertex_mask & SUMMIT_nde) >> 1) + ((vertex_mask & SUMMIT_sue) >> 4);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 2 + offset;
                    // (1,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 3 + offset;
                    // (0,0,0)
                    offset = (vertex_mask & SUMMIT_sde) + ((vertex_mask & SUMMIT_nde) >> 1);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + offset;
                    // (0,0,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + offset;
                    // (1,1,0)
                    offset = (vertex_mask & SUMMIT_sde) + ((vertex_mask & SUMMIT_nde) >> 1) + ((vertex_mask & SUMMIT_sue) >> 4);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 3 + offset;
                    // (1,0,0)
                    offset = (vertex_mask & SUMMIT_sde) + ((vertex_mask & SUMMIT_nde) >> 1);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 1 + offset;
                }
                if(face_mask & SUMMIT_u){
                    // (1,1,0)
                    offset = (vertex_mask & SUMMIT_sde) + ((vertex_mask & SUMMIT_nde) >> 1) + ((vertex_mask & SUMMIT_sdw) >> 2) + ((vertex_mask & SUMMIT_ndw) >> 3);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 3 + offset;
                    // (0,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 1 + offset;
                    // (1,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 2 + offset;
                    // (1,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 2 + offset;
                    // (0,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 1 + offset;
                    // (0,1,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 0 + offset;
                }
                if(face_mask & SUMMIT_n){
                    // (1,1,1)
                    offset = (vertex_mask & SUMMIT_sde) + ((vertex_mask & SUMMIT_sdw) >> 2) + ((vertex_mask & SUMMIT_sue) >> 4) + ((vertex_mask & SUMMIT_suw) >> 6);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 3 + offset;
                    // (1,0,1)
                    offset = (vertex_mask & SUMMIT_sde);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 0 + offset;
                    // (1,1,0)
                    offset = (vertex_mask & SUMMIT_sde) + ((vertex_mask & SUMMIT_sdw) >> 2) + ((vertex_mask & SUMMIT_sue) >> 4) + ((vertex_mask & SUMMIT_suw) >> 6);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 2 + offset;
                    // (1,1,0)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 2 + offset;
                    // (1,0,1)
                    offset = (vertex_mask & SUMMIT_sde);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 0 + offset;
                    // (1,0,0)
                    offset = (vertex_mask & SUMMIT_sde) + ((vertex_mask & SUMMIT_sdw) >> 2);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 1 + offset;
                }
                if(face_mask & SUMMIT_s){
                    // (0,1,1)
                    offset = ((vertex_mask & SUMMIT_nde) >> 1);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 2 + offset;
                    // (0,1,0)
                    offset = ((vertex_mask & SUMMIT_nde) >> 1) + ((vertex_mask & SUMMIT_ndw) >> 3);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 3 + offset;
                    // (0,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 0 + offset;
                    // (0,0,1)
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 0 + offset;
                    // (0,1,0)
                    offset = ((vertex_mask & SUMMIT_nde) >> 1) + ((vertex_mask & SUMMIT_ndw) >> 3);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 3 + offset;
                    // (0,0,0)
                    offset = ((vertex_mask & SUMMIT_nde) >> 1);
                    chunk->triangles_buffer[triangles_count++] = vertex_count - nb_vertex + 1 + offset;
                }
            }
        }
    }
    chunk->vertex_count = vertex_count;
    chunk->triangles_count = triangles_count;
}