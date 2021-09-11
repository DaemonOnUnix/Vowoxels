#ifndef VOXELENGINE_DATA_H
#define VOXELENGINE_DATA_H

#include "gl/camera.h"
#include "voxelengine/chunkmanager.h"
#include "voxelengine/texture.h"

typedef struct
{
    GLFWwindow* window;
    Camera* camera;
    unsigned int shaderProgram;
    struct Atlas* atlas;
    unsigned int height;
    unsigned int width;
} EngineData;

extern EngineData* data;

static inline EngineData* getEngineData(){
    return data;
}

#endif