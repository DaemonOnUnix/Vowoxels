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

bool RayCast(Ray ray, RaycastHit* hit);

#endif