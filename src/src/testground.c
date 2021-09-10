#include <stdio.h>
#include "log/log.h"
#include <stdlib.h>
#include "collections/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "voxelengine/chunk.h"
#include "voxelengine/chunk.h"

void tests(void* pika){
    (void)pika;
}

unsigned int test(){
    Chunk* chunk = calloc(1, sizeof(Chunk));
    chunk->voxel_list[0] = 1;
    chunk->voxel_list[1] = 1;
    updateChunkVertex(chunk);

	unsigned int VBO, VAO, IBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    // Index buffer
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->triangles_count * sizeof(unsigned int), chunk->triangles_buffer, GL_STATIC_DRAW);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * chunk->vertex_count, chunk->vertex_buffer, GL_STATIC_DRAW);
    
    return VAO;
}