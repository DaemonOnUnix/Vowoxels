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
#include "linear_algebra/perlinnoise.h"

int main() {
    

    data = malloc(sizeof(EngineData));
    data->atlas = malloc(sizeof(struct Atlas));
    data->atlas->next = NULL;
	data->width = 1280;
	data->height = 720;
    glinit();
    data->camera = initCamera(vec3$(5.0f, 5.0f,  5.0f), vec3$(0.0f, 0.0f, -1.0f), vec3$(0.0f, 1.0f,  0.0f));
	data->shaderProgram = bindShader();
    
    // Texture
    unsigned int atlasnb = createAtlas("testres/terrain.png", 16, 16, 0, 0, 3);
    setVoxelTileByIndex(atlasnb, 1, 3, FACE_SIDES);
    setVoxelTileByIndex(atlasnb, 1, 191, FACE_UP);
    setVoxelTileByIndex(atlasnb, 1, 2, FACE_DOWN);
    setVoxelTileByIndex(atlasnb, 2, 2, FACE_ALL);
    setVoxelTileByIndex(atlasnb, 3, 1, FACE_ALL);

    data->chunkM = initChunkManager();

	drawLoop();


    glDeleteTextures(1, &data->atlas->next->texture->m_texture);

	LOG_PANIC("End of program.");
	glend();
	return 0;
}