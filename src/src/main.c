#include <stdio.h>
#include "log/log.h"

#include <stdlib.h>
#include "collections/vector.h"
#include "linear_algebra/vec3.h"
#include "linear_algebra/mat4.h"
#include "linear_algebra/utils.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include "gl/manager.h"
#include "gl/camera.h"
#include "voxelengine/chunk.h"

#include "tests/testground.h"

#include "voxelengine/data.h"
EngineData* data;

int main() {
    data = malloc(sizeof(data));
    data->atlas = calloc(1, sizeof(struct Atlas));
	data->width = 640;
	data->height = 480;
    glinit();
    data->camera = initCamera(vec3$(0.0f, 0.0f,  3.0f), vec3$(0.0f, 0.0f, -1.0f), vec3$(0.0f, 1.0f,  0.0f));
	data->shaderProgram = bindShader();
    
    // Texture
    createAtlas("testres/terrain.png", 16, 16, 0, 0, NULL);
    data->atlas->next->tile_info[0].id = 2;
    data->atlas->next->tile_info[8].id = 1;
    data->atlas->next->tile_info[8].facemask = FACE_SIDES;
    data->atlas->next->tile_info[9].id = 1;
    data->atlas->next->tile_info[9].facemask = FACE_UP;
    data->atlas->next->tile_info[10].id = 1;
    data->atlas->next->tile_info[10].facemask = FACE_DOWN;

    data->chunkM = initChunkManager();

	drawLoop();


    glDeleteTextures(1, &data->atlas->next->texture->m_texture);

	LOG_PANIC("End of program.");
	glend();
	return 0;
}