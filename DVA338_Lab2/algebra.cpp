#define _USE_MATH_DEFINES // To get M_PI defined
#include <math.h>
#include <stdio.h>
#include "algebra.h"

const float PI = 3.14159265359f;

Vector CrossProduct(Vector a, Vector b) {
	Vector v = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
	return v;
}

float DotProduct(Vector a, Vector b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vector Subtract(Vector a, Vector b) {
	Vector v = { a.x-b.x, a.y-b.y, a.z-b.z };
	return v;
}    

Vector Add(Vector a, Vector b) {
	Vector v = { a.x+b.x, a.y+b.y, a.z+b.z };
	return v;
}    

float Length(Vector a) {
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

Vector Normalize(Vector a) {
	float len = Length(a);
	Vector v = { a.x/len, a.y/len, a.z/len };
	return v;
}

Vector ScalarVecMul(float t, Vector a) {
	Vector b = { t*a.x, t*a.y, t*a.z };
	return b;
}

HomVector MatVecMul(Matrix a, Vector b) {
	HomVector h;
	h.x = b.x*a.e[0] + b.y*a.e[4] + b.z*a.e[8] + a.e[12];
	h.y = b.x*a.e[1] + b.y*a.e[5] + b.z*a.e[9] + a.e[13];
	h.z = b.x*a.e[2] + b.y*a.e[6] + b.z*a.e[10] + a.e[14];
	h.w = b.x*a.e[3] + b.y*a.e[7] + b.z*a.e[11] + a.e[15];
	return h;
}

Vector Homogenize(HomVector h) {
	Vector a;
	if (h.w == 0.0) {
		fprintf(stderr, "Homogenize: w = 0\n");
		a.x = a.y = a.z = 9999999;
		return a;
	}
	a.x = h.x / h.w;
	a.y = h.y / h.w;
	a.z = h.z / h.w;
	return a;
}

Matrix MatMatMul(Matrix a, Matrix b) {
	Matrix c;
	int i, j, k;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			c.e[j*4+i] = 0.0;
			for (k = 0; k < 4; k++)
				c.e[j*4+i] += a.e[k*4+i] * b.e[j*4+k];
		}
	}
	return c;
}

void PrintVector(char const *name, Vector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z);
}

void PrintHomVector(char const *name, HomVector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z, a.w);
}

Matrix TranslationMatrix(Vector v)
{
	Matrix matrix = {
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		v.x, v.y, v.z, 1.0f
	};

	return matrix;
}

Matrix ScalingMatrix(Vector v)
{
	Matrix matrix = {
		v.x, 0.0f, 0.0f, 0.0f,
		0.0f, v.y, 0.0f, 0.0f,
		0.0f, 0.0f, v.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return matrix;
}

Matrix RotationMatrixX(float radians)
{
	Matrix matrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos(radians), sin(radians), 0.0f, 
		0.0f, -sin(radians), cos(radians), 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f 
	};
	return matrix;
}

Matrix RotationMatrixY(float radians)
{
	Matrix matrix = {
		cos(radians), 0.0f, -sin(radians), 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f, 
		sin(radians), 0.0f, cos(radians), 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f  
	};
	return matrix;
}

Matrix RotationMatrixZ(float radians)
{
	Matrix matrix = {
		cos(radians), sin(radians), 0.0f, 0.0f, 
		-sin(radians), cos(radians), 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f  
	};
	return matrix;
}

Matrix ViewMatrix(Vector c, Vector rotation)
{
	Vector negativeC = { -c.x, -c.y, -c.z };
	Matrix translation = TranslationMatrix(negativeC);

	float alphaRad = rotation.x * (PI / 180.0f);
	float betaRad = rotation.y * (PI / 180.0f);
	float gammaRad = rotation.z * (PI / 180.0f);
	Matrix rotationX = RotationMatrixX(-alphaRad);
	Matrix rotationY = RotationMatrixY(-betaRad);
	Matrix rotationZ = RotationMatrixZ(-gammaRad);

	Matrix tmp1 = MatMatMul(rotationX, translation);
	Matrix tmp2 = MatMatMul(rotationY, tmp1);
	Matrix V = MatMatMul(rotationZ, tmp2);

	return V;
}

void PrintMatrix(char const *name, Matrix a) { 
	int i,j;

	printf("%s:\n", name);
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf("%6.5lf ", a.e[j*4+i]);
		}
		printf("\n");
	}
}

Matrix PerspectiveMatrix(float fov, float aspect, float nearZ, float farZ) {
	Matrix m = { 0 }; 

	float f = 1.0f / tan(fov * (PI / 180.0f) / 2.0f);

	m.e[0] = f / aspect;
	m.e[5] = f;
	m.e[10] = -(farZ + nearZ) / (farZ - nearZ);
	m.e[11] = -1.0f;
	m.e[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);

	return m;
}

Matrix OrthographicMatrix(float left, float right, float bottom, float top, float nearZ, float farZ) {
	Matrix m = { 0 };

	m.e[0] = 2.0f / (right - left);
	m.e[5] = 2.0f / (top - bottom);
	m.e[10] = -2.0f / (farZ - nearZ);

	m.e[12] = -(right + left) / (right - left);
	m.e[13] = -(top + bottom) / (top - bottom);
	m.e[14] = -(farZ + nearZ) / (farZ - nearZ);
	m.e[15] = 1.0f;

	return m;
}

Vector SurfaceNormal(Vector a, Vector b, Vector c)
{
	Vector AB = Subtract(b, a);
	Vector AC = Subtract(c, a);
	Vector BC = Subtract(c, b);

	return CrossProduct(AB, AC);
}