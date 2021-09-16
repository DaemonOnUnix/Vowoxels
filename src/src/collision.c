#include "voxelengine/chunk.h"
#include "voxelengine/chunkmanager.h"
#include "voxelengine/collision.h"

bool RayCast(Ray ray, RaycastHit* hit){
    Vec3 start = vec3$(floorf(ray.origin.x), floorf(ray.origin.y), floorf(ray.origin.z));
    switch (ray.type)
    {
    case ALL:
        /* code */
        break;
    
    default:

        break;
    }
    return false;
}