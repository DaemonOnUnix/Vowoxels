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

void updateChunk(Chunk* chunk){
    if(!chunk->VAO){
        glGenVertexArrays(1, &chunk->VAO);
        glGenBuffers(2, chunk->VBO);
    }

    // Vertex buffer
    glBindVertexArray(chunk->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * chunk->vertex_count, chunk->vertex_buffer, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);
    // Texture Buffer
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(3 * sizeof(float)));
    // Normal Buffer
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(5 * sizeof(float)));


    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->VBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * chunk->triangles_count, chunk->triangles_buffer, GL_DYNAMIC_DRAW);

    glBindVertexArray(chunk->VAO);
}

void autoGenTileInfo(unsigned int atlasIndex){
    EngineData* data = getEngineData();
    struct Atlas* atlas = data->atlas;
    for (unsigned int i = 0; i < atlasIndex; i++) {
        atlas = atlas->next;
    }
    for (size_t id = 0; id < atlas->nb_voxels_id; id++) {
        for (size_t x = 0; x < 6; x++)
        {
            atlas->next->tile_info[id].tilex[x] = 0;
            atlas->next->tile_info[id].tiley[x] = 0;
        }
        
    }
}

unsigned int createAtlas(char* fileName, uint8_t tiles_x,  uint8_t tiles_y, uint8_t offset_x,  uint8_t offset_y, uint32_t nb_voxels_id){
    EngineData* data = getEngineData();
    unsigned int atlasIndex = 0;
    struct Atlas* atlas = data->atlas;
    while (atlas->next != NULL)
    {
        atlas = atlas->next;
        atlasIndex++;
    }
    
    // New atlas
    atlas->next = malloc(sizeof(struct Atlas));
    memset(atlas->next, 0, sizeof(struct Atlas));
    loadTexture(fileName, atlas->next);
    atlas->next->tilex = tiles_x;
    atlas->next->tiley = tiles_y;
    atlas->next->offset_x = offset_x;
    atlas->next->offset_y = offset_y;
    atlas->next->nb_voxels_id = nb_voxels_id;
    atlas->next->tile_info = malloc(sizeof(Tile) * nb_voxels_id);
    autoGenTileInfo(atlasIndex);
    LOG_OK("Created new atlas:\n\tId: %u\n\tTexture: %s\n\tTiles x: %u\n\tTiles y: %u\n\tOffset x: %u\n\tOffset y: %u", 
    atlasIndex, fileName, atlas->next->tilex, atlas->next->tiley, atlas->next->offset_x, atlas->next->offset_y)
    return atlasIndex;
}

void setVoxelTileByIndex(unsigned int atlasIndex, uint32_t voxel_id, uint32_t index, unsigned char faces){
    EngineData* data = getEngineData();
    struct Atlas* atlas = data->atlas;
    for (unsigned int i = 0; i <= atlasIndex; i++) {
        atlas = atlas->next;
    }
    if (voxel_id>atlas->nb_voxels_id || index > atlas->tilex * atlas->tiley)
    {
        return;
    }
    for (size_t i = 0; i < 6; i++)
    {
        if (faces & 1<<i)
        {
            atlas->tile_info[voxel_id].tilex[i] = index % atlas->tilex;
            atlas->tile_info[voxel_id].tiley[i] = index / atlas->tiley;
        }
        
    }
    
}
void setVoxelTileByCoord(unsigned int atlasIndex, uint32_t voxel_id, uint8_t tilex, uint8_t tiley, unsigned char faces){
    
    EngineData* data = getEngineData();
    struct Atlas* atlas = data->atlas;
    for (unsigned int i = 0; i <= atlasIndex; i++) {
        atlas = atlas->next;
    }
    if (voxel_id>atlas->nb_voxels_id || tilex > atlas->tilex || tiley > atlas->tiley)
    {
        return;
    }
    for (size_t i = 0; i < 6; i++)
    {
        if (faces & 1<<i)
        {
            atlas->tile_info[voxel_id].tilex[i] = tilex;
            atlas->tile_info[voxel_id].tiley[i] = tiley;
        }
        
    }
    
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