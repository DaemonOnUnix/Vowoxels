#ifndef OPENGL_CAMERA_H
#define OPENGL_CAMERA_H

#include "linear_algebra/vec3.h"

typedef struct
{
	// Camera
	Vec3 cameraPos;
	Vec3 cameraFront;
	Vec3 cameraUp;
    float cameraSpeed;
} Camera;

Camera* initCamera(Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp);

#endif