#include <stdint.h>
#include <stdio.h>
#include "linear_algebra/perlinnoise.h"
#include <stdlib.h>

static int SEED = 0;
static float poids[WORLD_SIZE][WORLD_SIZE][2];
void setSEED(int seed){
    SEED = seed;
    srand(SEED);

    for (size_t x = 0; x < WORLD_SIZE; x++)
    {
        for (size_t y = 0; y < WORLD_SIZE; y++)
        {
            switch (rand()%3)
            {
            case 0:
                poids[x][y][0] = 1.0f;
                poids[x][y][1] = 1.0f;
                break;
            case 1:
                poids[x][y][0] = -1.0f;
                poids[x][y][1] = 1.0f;
                break;
            case 2:
                poids[x][y][0] = -1.0f;
                poids[x][y][1] = -1.0f;
                break;
            default:
                poids[x][y][0] = 1.0f;
                poids[x][y][1] = -1.0f;
                break;
            }
        }
    }
    
    return;
}

float dotGradient(int32_t gridX, int32_t gridY, int32_t x, int32_t y){
    int32_t vecX = x - gridX;
    int32_t vecY = y - gridY;
    return vecX * poids[gridX][gridY][0] + vecY * poids[gridX][gridY][1];
}

float fade(float t){
    return ((6*t - 15)*t + 10)*t*t*t;
}

float lerp(float a0, float a1, float w){
    return (1.0f - w)*a0 + w*a1;
}

float noise2d(float x, float y){
    // X and Y MUST BE BETWEEN 0 WORLDSIZE
    if (x < 0)
        x = WORLD_SIZE + x;
    if (y < 0)
        y = WORLD_SIZE + y;
    
    float x0 = floorf(x);
    float x1 = x0 + 1;
    float y0 = floorf(y);
    float y1 = y0 + 1;

    float sx = x - x0;
    float sy = y - y0;

    float n0 = dotGradient(x0, y0, x, y);
    float n1 = dotGradient(x1, y0, x, y);
    float ix0 = lerp(n0, n1, fade(sx));
    n0 = dotGradient(x0, y1, x, y);
    n1 = dotGradient(x1, y1, x, y);
    float ix1 = lerp(n0, n1, fade(sx));
    return lerp(ix0, ix1, fade(sy));
}