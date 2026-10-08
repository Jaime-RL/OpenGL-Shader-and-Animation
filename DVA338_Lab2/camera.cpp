#include "camera.h"

Matrix PerspectiveProjectionMatrix(Camera cam, int screenWidth, int screenHeight)
{
	// Calculamos el aspect ratio de la ventana
	float aspect = (float)screenWidth / (float)screenHeight;

	// Usamos nuestra nueva función en algebra.cpp
	return PerspectiveMatrix(cam.fov, aspect, cam.nearPlane, cam.farPlane);
}

//Interface function for view matrix
Matrix ViewMatrix(Camera cam)
{
	// Assignment 1: Calculate the transform to view coordinates yourself 	
	// The matrix V should be calculated from camera parameters
	// Therefore, you need to replace this hard-coded transform. 
	Matrix V = ViewMatrix(cam.position, cam.rotation);
	
	return V;
}