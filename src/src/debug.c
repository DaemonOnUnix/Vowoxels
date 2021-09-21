#include "voxelengine/debug.h"
#include "log/log.h"
#include <GL/glew.h>
static struct DebugStruct debug_buffers;

void debugDrawBox(Vec3 start, Vec3 size, Vec3 color, float time){
    debugDrawLine(start, vec3_add(start, vec3$(size.x, 0.0f, 0.0f)), color, time);
    debugDrawLine(start, vec3_add(start, vec3$(0.0f, size.y, 0.0f)), color, time);
    debugDrawLine(start, vec3_add(start, vec3$(0.0f, 0.0f, size.z)), color, time);

    debugDrawLine(vec3_add(start, size), vec3_add(start, vec3$(size.x, 0.0f, size.z)), color, time);
    debugDrawLine(vec3_add(start, size), vec3_add(start, vec3$(0.0f, size.y, size.z)), color, time);
    debugDrawLine(vec3_add(start, size), vec3_add(start, vec3$(size.x, size.y, 0.0f)), color, time);

    debugDrawLine(vec3_add(start, vec3$(size.x, 0.0f, 0.0f)), vec3_add(start, vec3$(size.x, 0.0f, size.z)), color, time);
    debugDrawLine(vec3_add(start, vec3$(0.0f, 0.0f, size.z)), vec3_add(start, vec3$(size.x, 0.0f, size.z)), color, time);

    debugDrawLine(vec3_add(start, vec3$(0.0f, size.y, 0.0f)), vec3_add(start, vec3$(size.x, size.y, 0.0f)), color, time);
    debugDrawLine(vec3_add(start, vec3$(0.0f, size.y, 0.0f)), vec3_add(start, vec3$(0.0f, size.y, size.z)), color, time);

    debugDrawLine(vec3_add(start, vec3$(size.x, 0.0f, 0.0f)), vec3_add(start, vec3$(size.x, size.y, 0.0f)), color, time);
    debugDrawLine(vec3_add(start, vec3$(0.0f, 0.0f, size.z)), vec3_add(start, vec3$(0.0f, size.y, size.z)), color, time);
}

void debugDrawLine(Vec3 start, Vec3 end, Vec3 color, float time){
    addDebugVertex(start, color, time);
    addDebugVertex(end, color, time);
}

void addDebugVertex(Vec3 vertex, Vec3 color, float time){
    if (debug_buffers.vertexNumber > DEBUG_MAX_LINES)
    {
        LOG_ERR("Can't add debug line!");
        return;
    }
    debug_buffers.vertexBuffer[debug_buffers.vertexNumber].x = vertex.x;
    debug_buffers.vertexBuffer[debug_buffers.vertexNumber].y = vertex.y;
    debug_buffers.vertexBuffer[debug_buffers.vertexNumber].z = vertex.z;
    debug_buffers.vertexBuffer[debug_buffers.vertexNumber].color_r = color.x;
    debug_buffers.vertexBuffer[debug_buffers.vertexNumber].color_g = color.y;
    debug_buffers.vertexBuffer[debug_buffers.vertexNumber].color_b = color.z;
    debug_buffers.times[debug_buffers.vertexNumber] = time;
    debug_buffers.vertexNumber++;
}

unsigned int debugBindShader(){
    const char *vertexShaderSource = "#version 420 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 color;\n"
	"uniform mat4 model;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"
	"out vec3 Color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
	"	Color = color;\n"
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
	"out vec4 FragColor;\n"
	"in vec3 Color;\n"
	"void main()\n"
	"{\n"
	"    FragColor = vec4(Color,1);\n"
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

	debug_buffers.shaderprogram = glCreateProgram();

	glAttachShader(debug_buffers.shaderprogram, vertexShader);
	glAttachShader(debug_buffers.shaderprogram, fragmentShader);
	glLinkProgram(debug_buffers.shaderprogram);


    glGetProgramiv(debug_buffers.shaderprogram, GL_LINK_STATUS, &success);
	qASSERT(success);

	glUseProgram(debug_buffers.shaderprogram);
	return debug_buffers.shaderprogram;
}

void initDebug(){
    debugBindShader();
    if(!debug_buffers.VAO){
        glGenVertexArrays(1, &debug_buffers.VAO);
        glGenBuffers(1, &debug_buffers.VBO);
    }
    Vec3 m_pos = vec3$(0, 0, 0);
    Vec3 m_rot = vec3$(0, 0, 0);
    Vec3 m_scale = vec3$(1.0f, 1.0f, 1.0f);
    Mat4 posMatrix = mat4_translate(mat4_id(1.0f), m_pos);
    Mat4 rotXMatrix = mat4_rotate(mat4_id(1.0f), m_rot.x, vec3$(1,0,0));
    Mat4 rotYMatrix = mat4_rotate(mat4_id(1.0f), m_rot.y, vec3$(0,1,0));
    Mat4 rotZMatrix = mat4_rotate(mat4_id(1.0f), m_rot.z, vec3$(0,0,1));
    Mat4 scaleMatrix = mat4_scale(mat4_id(1.0f), m_scale);
    Mat4 rotMatrix = mat4_mult(rotZMatrix, mat4_mult(rotYMatrix, rotXMatrix));
	debug_buffers.model = mat4_mult(posMatrix, mat4_mult(rotMatrix, scaleMatrix));
    glLineWidth(5.0f);
}

void drawDebug(float delta_time, Mat4 view, Mat4 projection){
    uint32_t index = 0;
    for (uint32_t i = 0; i < debug_buffers.vertexNumber; i++)
    {
        debug_buffers.times[i] -= delta_time;
        if (index != i)
        {
            debug_buffers.times[index] = debug_buffers.times[i];
            debug_buffers.vertexBuffer[index] = debug_buffers.vertexBuffer[i];
        }
        if (debug_buffers.times[i] > 0)
            index++;
    }
    
	glUseProgram(debug_buffers.shaderprogram);
    if(!debug_buffers.VAO){
        glGenVertexArrays(1, &debug_buffers.VAO);
        glGenBuffers(1, &debug_buffers.VBO);
    }

    // Vertex buffer
    glBindVertexArray(debug_buffers.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, debug_buffers.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct DebugVertex) * index, debug_buffers.vertexBuffer, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct DebugVertex), (void*)0);
    // Texture Buffer
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct DebugVertex), (void*)(3 * sizeof(float)));
    


	int viewLoc = glGetUniformLocation(debug_buffers.shaderprogram, "view");
	int projectionLoc = glGetUniformLocation(debug_buffers.shaderprogram, "projection");
    int modelLoc = glGetUniformLocation(debug_buffers.shaderprogram, "model");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.data);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, debug_buffers.model.data);

    debug_buffers.vertexNumber = index;
	glBindVertexArray(debug_buffers.VAO);

    glDrawArrays(GL_LINES, 0, debug_buffers.vertexNumber);
}