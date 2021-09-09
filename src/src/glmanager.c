#include <stdio.h>
#include <stdlib.h>
#include "log/log.h"
#include "collections/vector.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "gl/manager.h"

GLFWwindow* window;

GLFWwindow* glinit(){
    qASSERT(glfwInit());
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(SCREEN_WITH, SCREEN_HEIGHT, "Playground", NULL, NULL);
    qASSERT(window);
	glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE; 
    ASSERT(!glewInit(), "Glew Init successful. (%s)", "Glew init failed with code %s", glewGetErrorString(glewInit()));
	glfwSetWindowTitle(window, "Playground");
	glEnable(GL_DEPTH_TEST);
    return window;
}

void glend(){
    glfwDestroyWindow(window);
	glfwTerminate();
}

unsigned int bindShader(){
    const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
	"uniform mat4 model;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\n";

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int success = 0;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        LOG_PANIC("ERROR::SHADER::PROGRAM::LINKING_FAILED\t%s", infoLog);
    }

	const char* fragmentShaderSource  = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	"}\n";

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        LOG_PANIC("ERROR::SHADER::PROGRAM::LINKING_FAILED\t%s", infoLog);
    }

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	qASSERT(success);

	glUseProgram(shaderProgram);
	return shaderProgram;
}