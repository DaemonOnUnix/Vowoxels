#include "voxelengine/chunk.h"
#include "voxelengine/chunkmanager.h"
#include "voxelengine/collision.h"

bool isRayCollideAABB(Ray r, float* inter_lenght, float minX, float maxX, float minY, float maxY, float minZ, float maxZ){
    Vec3 dirfrac;
    // r.dir is unit direction vector of ray
    dirfrac.x = 1.0f / r.dir.x;
    dirfrac.y = 1.0f / r.dir.y;
    dirfrac.z = 1.0f / r.dir.z;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    float t1 = (minX - r.origin.x)*r.dir.x;
    float t2 = (maxX - r.origin.x)*r.dir.x;
    float t3 = (minY - r.origin.y)*r.dir.y;
    float t4 = (maxY - r.origin.y)*r.dir.y;
    float t5 = (minZ - r.origin.z)*r.dir.z;
    float t6 = (maxZ - r.origin.z)*r.dir.z;

    float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
    float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    if (tmax < 0)
    {
        *inter_lenght = tmax;
        return false;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax)
    {
        *inter_lenght = tmax;
        return false;
    }

    *inter_lenght = tmin;
    return true;
}

bool RayCast(Ray ray, RaycastHit* hit){
    int32_t startX = (int32_t)floorf(ray.origin.x);
    int32_t startY = (int32_t)floorf(ray.origin.y);
    int32_t startZ = (int32_t)floorf(ray.origin.z);
    int32_t startChunkX = startX / CHUNK_DIMENSION;
    int32_t startChunkY = startY / CHUNK_DIMENSION;
    int32_t startChunkZ = startZ / CHUNK_DIMENSION;
    int32_t endChunkX = floorf(ray.origin.x + (ray.lenght * ray.dir.x)) / CHUNK_DIMENSION;
    int32_t endChunkY = floorf(ray.origin.y + (ray.lenght * ray.dir.y)) / CHUNK_DIMENSION;
    int32_t endChunkZ = floorf(ray.origin.z + (ray.lenght * ray.dir.z)) / CHUNK_DIMENSION;

    Chunk* chunk = getChunk(startChunkX, startChunkY, startChunkZ);
    if(!chunk)
        return false;

    char xdir = 1;
    if(ray.dir.x < 0){
        xdir = -1;
    }
    char ydir = 1;
    if(ray.dir.y < 0){
        ydir = -1;
    }
    char zdir = 1;
    if(ray.dir.z < 0){
        zdir = -1;
    }
    for (int32_t voxelx = startX - startChunkX; voxelx >= 0 && voxelx < CHUNK_DIMENSION; voxelx+=xdir)
    {
        for (int32_t voxely = startY - startChunkY; voxely >= 0 && voxely < CHUNK_DIMENSION; voxely+=ydir)
        {
            for (int32_t voxelz = startZ - startChunkZ; voxelz >= 0 && voxelz < CHUNK_DIMENSION; voxelz+=zdir)
            {
                float inter_lenght = 0;
                if(chunk->voxel_list[ INDEX_TO_CHUNK(voxelx, voxely, voxelz) ]){
                    if(isRayCollideAABB(ray, &inter_lenght, voxelx, voxelx+1.0f, voxely, voxely+1.0f, voxelz, voxelz+1.0f)){
                        hit->point = vec3$(voxelx, voxely, voxelz);
                        hit->type = CHUNK;
                        return true;
                    }
                }
            }
        }
    }


    return false;
}