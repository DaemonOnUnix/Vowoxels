#ifndef VOXELENGINE_TEXTURE_H
#define VOXELENGINE_TEXTURE_H

#include <stdint.h>

typedef struct
{
    uint32_t height;
    uint32_t width;
} Texture;

struct Atlas
{
    Texture* texture;
    uint8_t tilex;
    uint8_t tiley;
    struct Atlas* next;
};

// Return Atlas ID
unsigned int createAtlas(Texture* texture, uint8_t tiles_x,  uint8_t tiles_y);

#endif