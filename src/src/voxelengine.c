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

    int chunk_fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY | S_IRUSR | S_IWUSR);
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
            return 0;
        }
        if(!write(chunk_fd, &voxel_type, sizeof(int32_t))) {
            LOG_PANIC("Error to write voxel_type in chunk file '%s'", filename);
            return 0;
        }
        if(!write(chunk_fd, &voxel_mask, sizeof(int32_t))) {
            LOG_PANIC("Error to write voxel_mask in chunk file '%s'", filename);
            return 0;
        }
    }

    close(chunk_fd);
    return 1;
}

Chunk* loadChunkFromFile(char* dir, int32_t chunk_x, int32_t chunk_y, int32_t chunk_z){

    // Create filename
    smartstr pogstr filename = create_filename(dir, chunk_x, chunk_y, chunk_z);

    // Allocate Chunk
    Chunk* to_return = malloc(sizeof(Chunk));

    // Read Chunk
    int chunk_fd = open(filename, O_RDONLY);
    if(!chunk_fd) {
        LOG_PANIC("Error while loading chunk file '%s'", filename);
        return NULL;
    }
    smartstr pogstr _data = UNWRAP(pogstr, _string_from_fd(chunk_fd, 0));
    int32_t* data = (int32_t*) _data;
    close(chunk_fd);

    // Parse data
    size_t count = 0;
    size_t voxel_count = 0;
    for(size_t i = 0; count < CHUNK_NUM_VOXEL; i+=3) {
        int32_t num = data[i];
        int32_t voxel_type = data[i+1];
        int32_t voxel_mask = data[i+2];
        count += num;

        for(int32_t index = 0; index < num; index++) {
            to_return->voxel_list[voxel_count] = voxel_type;
            to_return->voxel_mask[voxel_count] = (enum voxel_mask)voxel_mask;
        }

    }

    return to_return;
}