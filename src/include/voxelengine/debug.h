#ifndef VOXELENGINE_DEBUG_H
#define VOXELENGINE_DEBUG_H

#include "linear_algebra/vec3.h"
#include "linear_algebra/mat4.h"
#include "voxelengine/collision.h"

#define DEBUG_MAX_LINES 100000

struct DebugVertex{
    float x;
    float y;
    float z;
    float color_r;
    float color_g;
    float color_b;
};

struct DebugStruct{
    uint32_t shaderprogram;
	unsigned int VAO;
	unsigned int VBO;
    Mat4 model;
    uint32_t vertexNumber;
    float times[DEBUG_MAX_LINES];
    struct DebugVertex vertexBuffer[DEBUG_MAX_LINES];
};

void initDebug();
void debugDrawBox(Vec3 start, Vec3 size, Vec3 color, float time);
void debugDrawLine(Vec3 start, Vec3 end, Vec3 color, float time);
static inline void debugDrawRay(Ray ray, Vec3 color, float time){
    debugDrawLine(ray.origin, vec3_add(ray.origin, vec3_mul_val(ray.dir, ray.lenght)), color, time);
}
void addDebugVertex(Vec3 vertex, Vec3 color, float time);
unsigned int debugBindShader();
void drawDebug(float delta_time, Mat4 view, Mat4 projection);

#endif