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

#include "tests/testground.h"

GLFWwindow* window;
Camera* cam;
unsigned int shaderProgram;

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

	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	qASSERT(success);

	glUseProgram(shaderProgram);
	return shaderProgram;
}

Camera* initCamera(Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp){
	cam = malloc(sizeof(Camera));
	cam->cameraPos   = cameraPos;
	cam->cameraFront = cameraFront;
	cam->cameraUp    = cameraUp;
	cam->cameraSpeed = 2.5f;
	return cam;
}

void processInput(float deltaTime){
	// Camera
	float cameraSpeed = cam->cameraSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam->cameraPos = vec3_add(cam->cameraPos, vec3_mul_val(cam->cameraFront, cameraSpeed));
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam->cameraPos = vec3_sub(cam->cameraPos, vec3_mul_val(cam->cameraFront, cameraSpeed));
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam->cameraPos = vec3_sub(cam->cameraPos, vec3_mul_val(vec3_unit(vec3_cross(cam->cameraFront, cam->cameraUp)), cameraSpeed));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam->cameraPos = vec3_add(cam->cameraPos, vec3_mul_val(vec3_unit(vec3_cross(cam->cameraFront, cam->cameraUp)), cameraSpeed));
}

void drawLoop(unsigned int VAO){
	float deltaTime = 0.0f; // Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame

	while (!glfwWindowShouldClose(window)){
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glUseProgram(shaderProgram);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//ProcessInput
  		processInput(deltaTime);
		

		// Le cube
        Mat4 view = mat4_lookAt(cam->cameraPos, vec3_sub(cam->cameraPos, cam->cameraFront), cam->cameraUp);
		Mat4 model = mat4_id(1.0f);
		model = mat4_rotate(model, (float)glfwGetTime(), vec3$(0.5f, 1.0f, 0.0f));
		//Mat4 view = mat4_id(1.0f);
		//view = mat4_translate(view, vec3$(0.0f, 0.0f, -3.0f));
		Mat4 projection;
		projection = mat4_perspective(to_radians(45.0f), (float)SCREEN_WITH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		int modelLoc = glGetUniformLocation(shaderProgram, "model");
		int viewLoc = glGetUniformLocation(shaderProgram, "view");
		int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.data);

		tests(window);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}