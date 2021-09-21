#include "voxelengine/chunk.h"
#include "voxelengine/chunkmanager.h"
#include "voxelengine/collision.h"
#include "voxelengine/debug.h"

bool isRayCollideAABB(Ray r, float* inter_lenght, float minX, float maxX, float minY, float maxY, float minZ, float maxZ){
    Vec3 dirfrac;
    // r.dir is unit direction vector of ray
    dirfrac.x = 1.0f / r.dir.x;
    dirfrac.y = 1.0f / r.dir.y;
    dirfrac.z = 1.0f / r.dir.z;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    float t1 = (minX - r.origin.x)*dirfrac.x;
    float t2 = (maxX - r.origin.x)*dirfrac.x;
    float t3 = (minY - r.origin.y)*dirfrac.y;
    float t4 = (maxY - r.origin.y)*dirfrac.y;
    float t5 = (minZ - r.origin.z)*dirfrac.z;
    float t6 = (maxZ - r.origin.z)*dirfrac.z;

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
    int32_t startChunkX = (int32_t)floorf((float)ray.origin.x / (float)CHUNK_DIMENSION);
    int32_t startChunkY = (int32_t)floorf((float)ray.origin.y / (float)CHUNK_DIMENSION);
    int32_t startChunkZ = (int32_t)floorf((float)ray.origin.z / (float)CHUNK_DIMENSION);
    int32_t endChunkX = (int32_t)floorf((float)(ray.origin.x + (ray.lenght * ray.dir.x)) / (float)CHUNK_DIMENSION);
    int32_t endChunkY = (int32_t)floorf((float)(ray.origin.y + (ray.lenght * ray.dir.y)) / (float)CHUNK_DIMENSION);
    int32_t endChunkZ = (int32_t)floorf((float)(ray.origin.z + (ray.lenght * ray.dir.z)) / (float)CHUNK_DIMENSION);

    // Dir to check
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
    debugDrawRay(ray, vec3$(1,1,1), 10);
    
    // Check Chunks
    for (int32_t chunkx = startChunkX; chunkx > endChunkX * (-xdir) && chunkx < endChunkX * xdir; chunkx+=xdir){
        for (int32_t chunky = startChunkY; chunky > endChunkY * (-ydir) &&  chunky < endChunkY * ydir; chunky+=ydir){
            for (int32_t chunkz = startChunkZ; chunkz > endChunkZ * (-zdir) &&  chunkz < endChunkZ * zdir; chunkz+=zdir){
            
                Chunk* chunk = getChunk(chunkx, chunky, chunkz);
                if(!chunk || chunk->is_air)
                    continue;
                Vec3 chunkcoord = vec3_mul_val(vec3$(chunkx, chunky, chunkz), CHUNK_DIMENSION);
                debugDrawBox(chunkcoord, vec3$(CHUNK_DIMENSION, CHUNK_DIMENSION, CHUNK_DIMENSION), vec3$(1,0,0), 10);

                // Check voxels
                // TODO: Optimize start value
                for (int32_t voxelx = (xdir==1 ? 0 : (CHUNK_DIMENSION-1)); voxelx >= 0 && voxelx < CHUNK_DIMENSION; voxelx+=xdir)
                {
                    for (int32_t voxely = (ydir==1 ? 0 : (CHUNK_DIMENSION-1)); voxely >= 0 && voxely < CHUNK_DIMENSION; voxely+=ydir)
                    {
                        for (int32_t voxelz = (zdir==1 ? 0 : (CHUNK_DIMENSION-1)); voxelz >= 0 && voxelz < CHUNK_DIMENSION; voxelz+=zdir)
                        {
                            if(chunk->voxel_list[ INDEX_TO_CHUNK(voxelx, voxely, voxelz) ]){
                                float inter_lenght = 0;
                                if(isRayCollideAABB(ray, &inter_lenght, chunkcoord.x+voxelx, chunkcoord.x+voxelx+1.0f, chunkcoord.y+voxely, chunkcoord.y+voxely+1.0f, chunkcoord.z+voxelz, chunkcoord.z+voxelz+1.0f)){
                                    debugDrawBox(vec3_add(chunkcoord, vec3$(voxelx, voxely, voxelz)), vec3$(1, 1, 1), vec3$(0,0,1), 10);
                                    hit->point = vec3$(voxelx, voxely, voxelz);
                                    hit->type = CHUNK;
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    



    return false;
}