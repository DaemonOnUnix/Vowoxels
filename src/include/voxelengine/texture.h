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
#define FACE_EAST    ((unsigned char)0b00000100)
#define FACE_WEST   ((unsigned char)0b00001000)
#define FACE_NORTH  ((unsigned char)0b00010000)
#define FACE_SOUTH  ((unsigned char)0b00100000)
#define FACE_ALL    ((unsigned char)0b00111111)

typedef struct
{
    uint32_t id;
    uint8_t facemask;
} Tile;


struct Atlas
{
    Texture* texture;
    uint8_t tilex;
    uint8_t tiley;
    Tile* tile_info;
    struct Atlas* next;
};

// Return Atlas ID
unsigned int createAtlas(char* fileName, uint8_t tiles_x,  uint8_t tiles_y, Tile* tile_info);
void loadTexture(char* fileName, struct Atlas*);

#endif