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

#include "tests/testground.h"

int main() {
    GLFWwindow* window = glinit();

	float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
	};
	unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); 

	unsigned int shader = bindShader();

	test();

	// Camera
	// Vec3 cameraPos   = vec3$(0.0f, 0.0f,  3.0f);
	// Vec3 cameraFront = vec3$(0.0f, 0.0f, -1.0f);
	// Vec3 cameraUp    = vec3$(0.0f, 1.0f,  0.0f);
	// float deltaTime = 0.0f; // Time between current frame and last frame
	// float lastFrame = 0.0f; // Time of last frame



	while (!glfwWindowShouldClose(window)){
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glUseProgram(shader);

		// float currentFrame = glfwGetTime();
		// deltaTime = currentFrame - lastFrame;
		// lastFrame = currentFrame;

		// ProcessInput
  		// float cameraSpeed = 2.5f * deltaTime;
		// if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		// 	vec3_add(cameraPos, vec3_mul_val(cameraFront, cameraSpeed));
		// if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		// 	vec3_sub(cameraPos, vec3_mul_val(cameraFront, cameraSpeed));
		// if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		// 	vec3_sub(cameraPos, vec3_mul_val(vec3_unit(vec3_cross(cameraFront, cameraUp)), cameraSpeed));
		// if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		// 	vec3_add(cameraPos, vec3_mul_val(vec3_unit(vec3_cross(cameraFront, cameraUp)), cameraSpeed));
		// mat4_lookAt(cameraPos, vec3_sub(cameraPos, cameraFront), cameraUp);
		

		// Le cube
		Mat4 model = mat4_id(1.0f);
		Mat4 view = mat4_id(1.0f);
		Mat4 projection;
		model = mat4_rotate(model, (float)glfwGetTime(), vec3$(0.5f, 1.0f, 0.0f));
		view = mat4_translate(view, vec3$(0.0f, 0.0f, -3.0f));
		projection = mat4_perspective(to_radians(45.0f), (float)SCREEN_WITH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		int modelLoc = glGetUniformLocation(shader, "model");
		int viewLoc = glGetUniformLocation(shader, "view");
		int projectionLoc = glGetUniformLocation(shader, "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.data);

		tests(window);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	LOG_PANIC("End of program.");
	glend();
	return 0;
}