#include <stdio.h>
#include <stdlib.h>
#include "log/log.h"
#include "collections/vector.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLFWwindow* window;

GLFWwindow* glinit(){
    qASSERT(glfwInit());
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(640, 480, "Playground", NULL, NULL);
    qASSERT(window);
	glfwMakeContextCurrent(window);
    ASSERT(!glewInit(), "Glew Init successful. (%s)", "Glew init failed with code %s", glewGetErrorString(glewInit()));
	glfwSetWindowTitle(window, "Playground");
    return window;
}

void glend(){
    glfwDestroyWindow(window);
	glfwTerminate();
}