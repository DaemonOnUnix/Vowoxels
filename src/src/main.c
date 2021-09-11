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

	unsigned int VBO, VAO, IBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);
    
    // Texture
    createAtlas("testres/terrain.png", 16, 16, NULL);

    // TEST CHUNK
	Chunk* chunk = calloc(1, sizeof(Chunk));
    for (size_t i = 0; i < 16*16; i++)
    {
        chunk->voxel_list[i*3] = i;
    }
    
    updateChunkVertex(chunk);



    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * chunk->vertex_count, chunk->vertex_buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture Buffer
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * chunk->triangles_count, chunk->triangles_buffer, GL_STATIC_DRAW);

    glBindVertexArray(VAO);

	drawLoop(VAO, chunk);


    glDeleteTextures(1, &data->atlas->next->texture->m_texture);

	LOG_PANIC("End of program.");
	glend();
	return 0;
}