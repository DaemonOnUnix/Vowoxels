#include <stdio.h>
#include "log/log.h"

#include <stdlib.h>
#include "collections/vector.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

int main() {
    qASSERT(glfwInit());
	// glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	// glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE,GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "My Window", NULL, NULL);
    qASSERT(window);
	glfwMakeContextCurrent(window);
    ASSERT(!glewInit(), "Glew Init successful. (%s)", "Glew init failed with code %s", glewGetErrorString(glewInit()));
	glfwSetWindowTitle(window, "Playground");
	glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	while (!glfwWindowShouldClose(window)){
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}