#include <stdio.h>
#include <stdlib.h>
#include "log/log.h"
#include "collections/vector.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "gl/manager.h"
#include "gl/camera.h"
#include "linear_algebra/vec3.h"
#include "linear_algebra/mat4.h"

#include "voxelengine/data.h"
#include "voxelengine/collision.h"
#include "voxelengine/debug.h"
#include "tests/testground.h"

GLFWwindow* glinit(){
	EngineData* data = getEngineData();
    qASSERT(glfwInit());
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    data->window = glfwCreateWindow(data->width, data->height, "Playground", NULL, NULL);
    qASSERT(data->window);
	glfwMakeContextCurrent(data->window);
    glewExperimental = GL_TRUE; 
    ASSERT(!glewInit(), "Glew Init successful. (%s)", "Glew init failed with code %s", glewGetErrorString(glewInit()));
	glfwSetWindowTitle(data->window, "Playground");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSetInputMode(data->window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(data->window, data->width/2.0f, data->height/2.0f);
	glfwSetWindowSizeCallback(data->window, window_size_callback);
    return data->window;
}

void glend(){
    glfwDestroyWindow(getEngineData()->window);
	glfwTerminate();
}

unsigned int bindShader(){
	EngineData* data = getEngineData();
    const char *vertexShaderSource = "#version 420 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 textCoord;\n"
    "layout (location = 2) in vec3 normal;\n"
	"uniform mat4 model;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"
	"out vec2 TexCoord;\n"
	"out vec3 FragPos;\n"
	"out vec3 Normal;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
	"   FragPos = vec3(model * vec4(aPos, 1.0));"
	"	TexCoord = textCoord;\n"
	"	Normal = normal;\n"
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

	const char* fragmentShaderSource  = "#version 420 core\n"
	"uniform sampler2D sampler;\n"
	"uniform vec3 skyLightDir;\n"
	"out vec4 FragColor;\n"
	"in vec2 TexCoord;\n"
	"in vec3 FragPos;\n"
	"in vec3 Normal;\n"
	"vec3 skyLightColor = vec3(1,1,1);\n"
	"void main()\n"
	"{\n"
	"	 float ambientStrength = 0.5;\n"
    " 	 vec3 ambient = ambientStrength * skyLightColor;\n"
	"	 vec3 norm = normalize(Normal);\n"
	"	 vec3 lightDir = normalize(-skyLightDir);\n"
	"    float diff = max(dot(norm, lightDir), 0.0);\n"
	"    vec3 diffuse = diff * skyLightColor;\n"
	"	 vec3 result = (ambient + diffuse);\n"
	"    FragColor = texture(sampler, TexCoord) * vec4(result,0);\n"
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

	data->shaderProgram = glCreateProgram();

	glAttachShader(data->shaderProgram, vertexShader);
	glAttachShader(data->shaderProgram, fragmentShader);
	glLinkProgram(data->shaderProgram);


    glGetProgramiv(data->shaderProgram, GL_LINK_STATUS, &success);
	qASSERT(success);

	glUseProgram(data->shaderProgram);
	return data->shaderProgram;
}

Camera* initCamera(Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp){
	Camera* cam = malloc(sizeof(Camera));
	cam->cameraPos   = cameraPos;
	cam->cameraFront = cameraFront;
	cam->cameraUp    = cameraUp;
	cam->cameraSpeed = 100.0f;
	return cam;
}

void drawChunk(Chunk* chunk){
	if(!chunk || chunk->is_air)
		return;
	glUseProgram(data->shaderProgram);
	if (chunk->need_vertex_update)
	{
		updateChunkVertex(chunk);
		updateChunk(chunk);
		chunk->need_vertex_update = false;
	}
	
	if(!chunk->VAO){
		updateChunk(chunk);
	}
	int modelLoc = glGetUniformLocation(data->shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, chunk->model.data);
	glBindVertexArray(chunk->VAO);
	glActiveTexture(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, chunk->triangles_count, GL_UNSIGNED_INT, (void*)0);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
	UNUSED(window);
	EngineData* data = getEngineData();
	data->width = width;
	data->height = height;
	glViewport(0, 0, width, height);
}