#include "collections/string.h"
#include "voxelengine/chunk.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>

Chunk* newChunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z){
    Chunk* chunk = calloc(1, sizeof(Chunk));
    chunk->VAO = 0;

    chunk->chunk_x = chunk_x;
    chunk->chunk_y = chunk_y;
    chunk->chunk_z = chunk_z;

    chunk->transform.m_pos = vec3_mul_val(vec3$(chunk_x, chunk_y, chunk_z), CHUNK_DIMENSION);
    chunk->transform.m_rot = vec3$(0, 0, 0);
    chunk->transform.m_scale = vec3$(1.0f, 1.0f, 1.0f);

    Mat4 posMatrix = mat4_translate(mat4_id(1.0f), chunk->transform.m_pos);
    Mat4 rotXMatrix = mat4_rotate(mat4_id(1.0f), chunk->transform.m_rot.x, vec3$(1,0,0));
    Mat4 rotYMatrix = mat4_rotate(mat4_id(1.0f), chunk->transform.m_rot.y, vec3$(0,1,0));
    Mat4 rotZMatrix = mat4_rotate(mat4_id(1.0f), chunk->transform.m_rot.z, vec3$(0,0,1));
    Mat4 scaleMatrix = mat4_scale(mat4_id(1.0f), chunk->transform.m_scale);

    Mat4 rotMatrix = mat4_mult(rotZMatrix, mat4_mult(rotYMatrix, rotXMatrix));

	chunk->model = mat4_mult(posMatrix, mat4_mult(rotMatrix, scaleMatrix));
    chunk->is_air = false;
    chunk->triangles_buffer = NULL;
    chunk->vertex_buffer = NULL;
    LOG_OK("Create chunk %i %i %i", chunk_x, chunk_y, chunk_z)
    return chunk;
}

void freeChunk(Chunk* chunk){
    if (!chunk)
        return;
    if(chunk->VAO){
        glDeleteVertexArrays(1, &chunk->VAO);
        glDeleteBuffers(2, chunk->VBO);
    }
    if(chunk->triangles_buffer)
        free(chunk->triangles_buffer);
    if(chunk->vertex_buffer)
        free(chunk->vertex_buffer);
    free(chunk);
    return;
}

unsigned char saveChunkToFile(Chunk* chunk, char* dir){
    
    // Create filename
    smartstr pogstr filename = create_chunk_filename(dir, chunk->chunk_x, chunk->chunk_y, chunk->chunk_z);

    int chunk_fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if(!chunk_fd) {
        LOG_PANIC("Error while saving chunk file '%s'", filename);
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
    smartstr pogstr filename = create_chunk_filename(dir, chunk_x, chunk_y, chunk_z);

    // Read Chunk
    int chunk_fd = open(filename, O_RDONLY);
    if(chunk_fd <= 0) {
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

pogstr create_chunk_filename(char* dir, int32_t chunk_x, int32_t chunk_y, int32_t chunk_z){
    // Create filename
    pogstr filename = string(dir);
    char buffer[50] = {0};
    sprintf(buffer, "%u_%u_%u", (uint32_t)chunk_x, (uint32_t)chunk_y, (uint32_t)chunk_z);
    filename = _str_cat(filename, string(buffer));
    return filename;
}