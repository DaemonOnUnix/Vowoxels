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
    createAtlas("testres/terrain.png", 16, 16, 0, 0, NULL);
    data->atlas->next->tile_info[0].id = 2;
    data->atlas->next->tile_info[8].id = 1;
    data->atlas->next->tile_info[8].facemask = FACE_SIDES;
    data->atlas->next->tile_info[9].id = 1;
    data->atlas->next->tile_info[9].facemask = FACE_UP;
    data->atlas->next->tile_info[10].id = 1;
    data->atlas->next->tile_info[10].facemask = FACE_DOWN;

    // TEST CHUNK
	Chunk* chunk = calloc(1, sizeof(Chunk));
    for (size_t x = 0; x < CHUNK_DIMENSION; x++)
    {
        for (size_t y = 0; y < CHUNK_DIMENSION; y++)
        {
            for (size_t z = 0; z < CHUNK_DIMENSION; z++)
            {
                if (y <= 0){
                    chunk->voxel_list[INDEX_TO_CHUNK(x, y, z)] = 1;
                }
            }
            
        }
        
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