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
#include "tests/testground.h"

bool viewMode = false;
bool firstMouse = true;
float lastX = 400;
float lastY = 300;
float yaw;
float pitch;

GLFWwindow* glinit(){
	EngineData* data = getEngineData();
    qASSERT(glfwInit());
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    data->window = glfwCreateWindow(SCREEN_WITH, SCREEN_HEIGHT, "Playground", NULL, NULL);
    qASSERT(data->window);
	glfwMakeContextCurrent(data->window);
    glewExperimental = GL_TRUE; 
    ASSERT(!glewInit(), "Glew Init successful. (%s)", "Glew init failed with code %s", glewGetErrorString(glewInit()));
	glfwSetWindowTitle(data->window, "Playground");
	glEnable(GL_DEPTH_TEST);
	//glfwSetInputMode(data->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(data->window, mouse_callback);
    return data->window;
}

void glend(){
    glfwDestroyWindow(getEngineData()->window);
	glfwTerminate();
}

unsigned int bindShader(){
	EngineData* data = getEngineData();
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
	cam->cameraSpeed = 2.5f;
	return cam;
}

void processInput(float deltaTime){
	EngineData* data = getEngineData();
	// Camera
	float cameraSpeed = data->camera->cameraSpeed * deltaTime;
	if (glfwGetKey(data->window, GLFW_KEY_W) == GLFW_PRESS)
		data->camera->cameraPos = vec3_add(data->camera->cameraPos, vec3_mul_val(data->camera->cameraFront, cameraSpeed));
	if (glfwGetKey(data->window, GLFW_KEY_S) == GLFW_PRESS)
		data->camera->cameraPos = vec3_sub(data->camera->cameraPos, vec3_mul_val(data->camera->cameraFront, cameraSpeed));
	if (glfwGetKey(data->window, GLFW_KEY_A) == GLFW_PRESS)
		data->camera->cameraPos = vec3_sub(data->camera->cameraPos, vec3_mul_val(vec3_unit(vec3_cross(data->camera->cameraFront, data->camera->cameraUp)), cameraSpeed));
	if (glfwGetKey(data->window, GLFW_KEY_D) == GLFW_PRESS)
		data->camera->cameraPos = vec3_add(data->camera->cameraPos, vec3_mul_val(vec3_unit(vec3_cross(data->camera->cameraFront, data->camera->cameraUp)), cameraSpeed));
	if (glfwGetKey(data->window, GLFW_KEY_P) == GLFW_PRESS){
		if (viewMode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			viewMode = !viewMode;
		}
		else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			viewMode = !viewMode;
		}
	}
		
}

void drawLoop(unsigned int VAO, Chunk* chunk){
	EngineData* data = getEngineData();

	float deltaTime = 0.0f; // Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame

	while (!glfwWindowShouldClose(data->window)){
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glUseProgram(data->shaderProgram);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//ProcessInput
  		processInput(deltaTime);
		

		// Le cube
        Mat4 view = mat4_lookAt(data->camera->cameraPos, vec3_sub(data->camera->cameraPos, data->camera->cameraFront), data->camera->cameraUp);
		Mat4 model = mat4_id(1.0f);
		model = mat4_rotate(model, (float)glfwGetTime(), vec3$(0.5f, 1.0f, 0.0f));
		Mat4 projection;
		projection = mat4_perspective(to_radians(45.0f), (float)SCREEN_WITH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		int modelLoc = glGetUniformLocation(data->shaderProgram, "model");
		int viewLoc = glGetUniformLocation(data->shaderProgram, "view");
		int projectionLoc = glGetUniformLocation(data->shaderProgram, "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.data);

		tests(data->window);
		
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, chunk->triangles_count, GL_UNSIGNED_INT, (void*)0);

		glfwSwapBuffers(data->window);
		glfwPollEvents();
	}
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	UNUSED(window);
	if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // il faut inverser car ypos est donné de haut en bas
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw   += xoffset;
	pitch += yoffset;

	if(pitch > 89.0f)
		pitch =  89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	Vec3 front = vec3$(cos(to_radians(pitch)) * cos(to_radians(yaw)), sin(to_radians(pitch)), cos(to_radians(pitch)) * sin(to_radians(yaw)));
	getEngineData()->camera->cameraFront = vec3_unit(front);
}