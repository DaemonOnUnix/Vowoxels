#ifndef VOXELENGINE_DATA_H
#define VOXELENGINE_DATA_H

#include "gl/camera.h"
#include "voxelengine/chunkmanager.h"
#include "voxelengine/texture.h"

typedef struct
{
    GLFWwindow* window;
    Camera* camera;
    Chunk_manager* chunkM;
    struct Atlas *atlas;
    uint32_t shaderProgram;
    uint16_t height;
    uint16_t width;
} EngineData;

extern EngineData* data;

static inline EngineData* getEngineData(){
    return data;
}

#endif