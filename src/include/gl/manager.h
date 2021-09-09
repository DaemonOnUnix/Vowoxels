#ifndef OPENGL_MANAGER_H
#define OPENGL_MANAGER_H

#include <GLFW/glfw3.h>

#define SCREEN_WITH 	640
#define SCREEN_HEIGHT 	480

GLFWwindow* glinit();
void glend();
unsigned int bindShader();

#endif