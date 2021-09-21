#ifndef VOXELENGINE_COLLISION_H
#define VOXELENGINE_COLLISION_H

#include "linear_algebra/vec3.h"

typedef enum {
    ALL,
    CHUNK,
    PLAYER
} ObjectType;

typedef struct
{
    ObjectType type;
    Vec3 point;
} RaycastHit;

typedef struct
{
    Vec3 origin;
    Vec3 dir;
    float lenght;
    ObjectType type;
} Ray;

bool isRayCollideAABB(Ray r, float* inter_lenght, float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
static inline bool isPointInsideAABB(Vec3 point, float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
  return (point.x >= minX && point.x <= maxX) &&
         (point.y >= minY && point.y <= maxY) &&
         (point.z >= minZ && point.z <= maxZ);
}

bool RayCast(Ray ray, RaycastHit* hit);

#endif