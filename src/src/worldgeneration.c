#include "collections/string.h"
#include "voxelengine/chunk.h"
#include "voxelengine/data.h"
#include "voxelengine/chunkmanager.h"
#include "linear_algebra/perlinnoise.h"

void updateCoord(int* dir, int32_t* x, int32_t* y, int32_t* z, int32_t camx, int32_t camy, int32_t camz){
    EngineData* data = getEngineData();
    if(*y<0){
        LOG_INFO("UNDER THE MAP")
        *dir = 5;
    }
    switch (*dir)
    {
    case 0:
        (*y) = camy+VIEW_DIST;
        (*x)++;
        (*dir)++;
        break;
    case 1:
        (*y)--;
        if(*y<=0){
            (*y) = camy+VIEW_DIST;
            (*dir)++;
        }
        break;
    case 2:
        if((*y) == 0){
            (*y) = camy+VIEW_DIST;
            (*z)++;
            if (*z - camz == *x - camx)
                (*dir)++;
        }else{
            (*y)--;
        }
        break;
    case 3:
        if((*y) == 0){
            (*y) = camy+VIEW_DIST;
            (*x)--;
            if (*x - camx == -(*z - camz))
                (*dir)++;
        }else{
            (*y)--;
        }
        break;
    case 4:
        if((*y) == 0){
            (*y) = camy+VIEW_DIST;
            (*z)--;
            if (*x - camx == *z - camz)
                (*dir)++;
        }else{
            (*y)--;
        }
        break;
    case 5:
        if((*y) == 0){
            (*y) = camy+VIEW_DIST;
            (*x)++;
            if (*x - camx == -(*z - camz))
                (*dir)++;
        }else{
            (*y)--;
        }
        break;
    case 6:
        (*y)--;
        if((*y) == 0){
            if(VIEW_DIST > *x - camx)
                (*dir)=0;
            else
                (*dir)++;
        }
        break;
    case 7:
        LOG_INFO("Finish to load all CHUNKS")
        while(!__sync_bool_compare_and_swap(&(data->chunkM->need_update), 1, 2));
        __sync_synchronize();
        break;
    default:
        PANIC("How did you get there ?");
        break;
    }
    LOG_INFO("Updating Coord x: %i; y: %i; z: %i, dir: %i", *x, *y, *z, *dir)
}

Chunk* generateChunk(int32_t x, int32_t y, int32_t z){
    Chunk* chunk = newChunk(x, y, z);
    for (size_t _x = 0; _x < CHUNK_DIMENSION; _x++)
    {
        float lx = ((float)(_x+0.5f)/(float)CHUNK_DIMENSION) + (x+(WORLD_SIZE/2));
        for (size_t _z = 0; _z < CHUNK_DIMENSION; _z++)
        {
            float lz = ((float)(_z+0.5f)/(float)CHUNK_DIMENSION) + (z+(WORLD_SIZE/2));
            float p = noise2d(lx, lz);
            p = floorf(p * MAX_HEIGHT);
            if (p < 0)
                p *= -1;
            if(p > MAX_HEIGHT)
                p = MAX_HEIGHT;
            for (size_t _y = 0; _y < CHUNK_DIMENSION && ((y*CHUNK_DIMENSION)+_y < p); _y++)
            {
                // GRASS
                chunk->voxel_list[INDEX_TO_CHUNK(_x, _y, _z)] = 1;
                // DIRT
                if ((y*CHUNK_DIMENSION)+_y < p-1)
                    chunk->voxel_list[INDEX_TO_CHUNK(_x, _y, _z)] = 2;
                // STONE
                if ((y*CHUNK_DIMENSION)+_y < p-5)
                    chunk->voxel_list[INDEX_TO_CHUNK(_x, _y, _z)] = 3;
            }
            
        }
        
    }
    return chunk;
}