#ifndef VOXELENGINE_TEXTURE_H
#define VOXELENGINE_TEXTURE_H

#include <stdint.h>

typedef struct
{
    unsigned int m_texture;
    uint32_t height;
    uint32_t width;
    int mode;
} Texture;


#define FACE_UP     ((unsigned char)0b00000001)
#define FACE_DOWN   ((unsigned char)0b00000010)
#define FACE_EAST   ((unsigned char)0b00000100)
#define FACE_WEST   ((unsigned char)0b00001000)
#define FACE_NORTH  ((unsigned char)0b00010000)
#define FACE_SOUTH  ((unsigned char)0b00100000)
#define FACE_ALL    ((unsigned char)0b00111111)
#define FACE_SIDES  ((unsigned char)0b00111100)

typedef struct
{
    uint8_t tilex[6];
    uint8_t tiley[6];
} Tile;


struct Atlas
{
    Texture* texture;
    uint8_t tilex;
    uint8_t tiley;
    uint8_t offset_x;
    uint8_t offset_y;
    uint32_t nb_voxels_id;
    Tile* tile_info;
    struct Atlas* next;
};

// Return Atlas ID
unsigned int createAtlas(char* fileName, uint8_t tiles_x,  uint8_t tiles_y, uint8_t offset_x, uint8_t offset_y, uint32_t nb_voxels_id);
void loadTexture(char* fileName, struct Atlas*);
void setVoxelTileByIndex(unsigned int atlasIndex, uint32_t voxel_id, uint32_t index, unsigned char faces);
void setVoxelTileByCoord(unsigned int atlasIndex, uint32_t voxel_id, uint8_t tilex, uint8_t tiley, unsigned char faces);
#endif