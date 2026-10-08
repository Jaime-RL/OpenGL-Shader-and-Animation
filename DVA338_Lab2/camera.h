#ifndef _CAMERA_H
#define _CAMERA_H
#include "algebra.h"

typedef struct _Camera {
	Vector position;
	Vector rotation;
	double fov; 
	double nearPlane; 
	double farPlane; 
} Camera;

Matrix PerspectiveProjectionMatrix(
	Camera cam, 
	int screenWidth, 
	int screenHeight);
Matrix ViewMatrix(Camera cam);

#endif