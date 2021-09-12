#include <GL/glew.h>
#include "voxelengine/chunk.h"
#include "voxelengine/texture.h"
#include "voxelengine/data.h"
#include "collections/string.h"
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>

#ifndef bool
#define bool unsigned char
#endif

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
    LOG_OK("Create chunk %i %i %i", chunk_x, chunk_y, chunk_z)
    return chunk;
}

pogstr create_filename(char* dir, int32_t chunk_x, int32_t chunk_y, int32_t chunk_z){
    // Create filename
    pogstr filename = string(dir);
    char buffer[50] = {0};
    sprintf(buffer, "%u_%u_%u", (uint32_t)chunk_x, (uint32_t)chunk_y, (uint32_t)chunk_z);
    filename = _str_cat(filename, string(buffer));
    return filename;
}

unsigned char saveChunkToFile(Chunk* chunk, char* dir){
    
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

#define VERTEX_BUFFER_E(_x, _y, _z, _uvx, _uvy) chunk->vertex_buffer[vertex_count].x = _x;\
    chunk->vertex_buffer[vertex_count].y = _y;\
    chunk->vertex_buffer[vertex_count].z = _z;\
    chunk->vertex_buffer[vertex_count].uv_x = (_uvx);\
    chunk->vertex_buffer[vertex_count].uv_y = (_uvy);\
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
                    VERTEX_BUFFER_E(x, y, z+1, xuv+offsetXPerc, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x, y, z, xuv, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x, y+1, z+1, xuv+offsetXPerc, yuv);
                    VERTEX_BUFFER_E(x, y+1, z, xuv, yuv);

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
                    VERTEX_BUFFER_E(x+1, y+1, z+1, xuv, yuv);
                    VERTEX_BUFFER_E(x+1, y, z+1, xuv, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x+1, y, z, xuv+offsetXPerc, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x+1, y+1, z, xuv+offsetXPerc, yuv);
                    
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
                    VERTEX_BUFFER_E(x, y, z+1, xuv+offsetXPerc, yuv);
                    VERTEX_BUFFER_E(x+1, y, z+1, xuv+offsetXPerc, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x, y, z, xuv, yuv);
                    VERTEX_BUFFER_E(x+1, y, z, xuv, yuv+offsetYPerc);
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
                    VERTEX_BUFFER_E(x+1, y+1, z+1, xuv+offsetXPerc, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x, y+1, z+1, xuv, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x, y+1, z, xuv, yuv);
                    VERTEX_BUFFER_E(x+1, y+1, z, xuv+offsetXPerc, yuv);
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
                    VERTEX_BUFFER_E(x, y, z, xuv+offsetXPerc, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x+1, y, z, xuv, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x, y+1, z, xuv+offsetXPerc, yuv);
                    VERTEX_BUFFER_E(x+1, y+1, z, xuv, yuv);
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
                    VERTEX_BUFFER_E(x, y, z+1, xuv, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x+1, y, z+1, xuv+offsetXPerc, yuv+offsetYPerc);
                    VERTEX_BUFFER_E(x, y+1, z+1, xuv, yuv);
                    VERTEX_BUFFER_E(x+1, y+1, z+1, xuv+offsetXPerc, yuv);
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
}

void updateChunk(Chunk* chunk){
    if(!chunk->VAO){
        glGenVertexArrays(1, &chunk->VAO);
        glGenBuffers(2, chunk->VBO);
    }

    // Vertex buffer
    glBindVertexArray(chunk->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * chunk->vertex_count, chunk->vertex_buffer, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    // Texture Buffer
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));


    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->VBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * chunk->triangles_count, chunk->triangles_buffer, GL_STATIC_DRAW);

    glBindVertexArray(chunk->VAO);
}

void autoGenTileInfo(unsigned int atlasIndex){
    EngineData* data = getEngineData();
    struct Atlas* atlas = data->atlas;
    for (unsigned int i = 0; i < atlasIndex; i++) {
        atlas = atlas->next;
    }
    unsigned int nb_blc = atlas->next->tilex * atlas->next->tiley;
    atlas->next->tile_info = malloc(sizeof(Tile) * nb_blc);
    for (size_t id = 0; id < nb_blc; id++) {
        atlas->next->tile_info[id].id = (unsigned int)(id+1);
        atlas->next->tile_info[id].facemask = FACE_ALL;
    }
}

unsigned int createAtlas(char* fileName, uint8_t tiles_x,  uint8_t tiles_y, uint8_t offset_x,  uint8_t offset_y, Tile* tile_info){
    EngineData* data = getEngineData();
    unsigned int atlasIndex = 0;
    struct Atlas* atlas = data->atlas;
    for (; atlas->next != NULL; atlas = atlas->next, atlasIndex++);
    
    // New atlas
    atlas->next = malloc(sizeof(struct Atlas));
    memset(atlas->next, 0, sizeof(struct Atlas));
    loadTexture(fileName, atlas->next);
    atlas->next->tilex = tiles_x;
    atlas->next->tiley = tiles_y;
    atlas->next->offset_x = offset_x;
    atlas->next->offset_y = offset_y;
    char isAutoGen[10];
    memcpy(isAutoGen, "False", 6);
    if (tile_info)
        atlas->tile_info = tile_info;
    else{
        memcpy(isAutoGen, "True", 5);
        autoGenTileInfo(atlasIndex);
    }
    LOG_OK("Created new atlas:\n\tId: %u\n\tTexture: %s\n\tTiles x: %u\n\tTiles y: %u\n\tOffset x: %u\n\tOffset y: %u\n\tAutoGen: %s", 
    atlasIndex, fileName, atlas->next->tilex, atlas->next->tiley, atlas->next->offset_x, atlas->next->offset_y, isAutoGen)
    return atlasIndex;
}

void loadTexture(char* fileName, struct Atlas* atlas){
    SDL_Surface* image_surface = IMG_Load(fileName);
    if (!image_surface) {
        PANIC("Failed to load texture: %s", fileName);
        return;
    }
    atlas->texture = malloc(sizeof(Texture));
    char mode[6];
    memcpy(mode, "RGB", 4);
    atlas->texture->mode = GL_RGB;
    if(image_surface->format->BitsPerPixel>=4){
        memcpy(mode, "RGBA", 5);
        atlas->texture->mode = GL_RGBA;
    }
    atlas->texture->height = image_surface->h;
    atlas->texture->width = image_surface->w;

    LOG_OK("Successfuly load texture:\n\tName: \"%s\"\n\tWidth: %u\n\tHeight: %u\n\tMode: %s", fileName, atlas->texture->width, atlas->texture->height, mode)

    glGenTextures(1, &atlas->texture->m_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas->texture->m_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_surface->w, image_surface->h, 0, atlas->texture->mode, GL_UNSIGNED_BYTE, image_surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    LOG_INFO("Texture bind to atlas")

    SDL_FreeSurface(image_surface);
}
