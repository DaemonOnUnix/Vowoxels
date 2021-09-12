#ifndef OPENGL_MANAGER_H
#define OPENGL_MANAGER_H

#include <GLFW/glfw3.h>
#include "voxelengine/chunk.h"

GLFWwindow* glinit();
void glend();
unsigned int bindShader();
void processInput(float deltaTime);
void drawLoop();
void window_size_callback(GLFWwindow* window, int width, int height);

#endif