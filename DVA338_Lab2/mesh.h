#ifndef _MESH_H_
#define _MESH_H_

#include "algebra.h"

typedef struct _Triangle {
	int vInds[3]; //vertex indices
} Triangle;

typedef struct _Mesh { 
	int nv;				
	Vector *vertices;
	Vector *vnorms;
	int nt;				
	Triangle *triangles;
	struct _Mesh *next; 

	// INDICAMOS COMO CREAR CADA OBJETO
	Vector translation;
	Vector scaling;
	Vector rotation;
		
	unsigned int vbo, ibo, vao; // OpenGL handles for rendering
} Mesh;

void insertModel(Mesh** list, int nv, float* vArr, int nt, int* tArr, Vector t, Vector s, Vector r);

#endif
