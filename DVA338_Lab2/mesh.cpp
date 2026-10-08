#include <stdlib.h>
#include "mesh.h"

void insertModel(Mesh** list, int nv, float* vArr, int nt, int* tArr, Vector t, Vector s, Vector r) {
	Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
	mesh->nv = nv;
	mesh->nt = nt;
	mesh->vertices = (Vector*)malloc(nv * sizeof(Vector));
	// USE CALLOC TO START WITH EMPTY MEMORY
	mesh->vnorms = (Vector*)calloc(nv, sizeof(Vector));
	mesh->triangles = (Triangle*)malloc(nt * sizeof(Triangle));

	mesh->translation = t;
	mesh->scaling = s;
	mesh->rotation = r;

	// set mesh vertices
	for (int i = 0; i < nv; i++) {
		mesh->vertices[i].x = vArr[i * 3];
		mesh->vertices[i].y = vArr[i * 3 + 1];
		mesh->vertices[i].z = vArr[i * 3 + 2];
	}

	// set mesh triangles
	for (int i = 0; i < nt; i++) {
		mesh->triangles[i].vInds[0] = tArr[i * 3];
		mesh->triangles[i].vInds[1] = tArr[i * 3 + 1];
		mesh->triangles[i].vInds[2] = tArr[i * 3 + 2];
	}

	// Assignment 1: 
	// Calculate and store suitable vertex normals for the mesh here.
	// Replace the code below that simply sets some arbitrary normal values	
	// Hint: You need to fix the implementation of the SurfaceNormal function
	// and use that for normal calculations.
	for (int i = 0; i < nt; i++) {
		
		// GET VERTICES IDS
		int idxA = mesh->triangles[i].vInds[0];
		int idxB = mesh->triangles[i].vInds[1];
		int idxC = mesh->triangles[i].vInds[2];

		// GET VERTICES
		Vector A = mesh->vertices[idxA];
		Vector B = mesh->vertices[idxB];
		Vector C = mesh->vertices[idxC];

		// CALCULATE NORMAL
		Vector faceNormal = SurfaceNormal(A, B, C);

		// ACUMULATE THE NORMAL FOR EACH VERTICE
		mesh->vnorms[idxA] = Add(mesh->vnorms[idxA], faceNormal);
		mesh->vnorms[idxB] = Add(mesh->vnorms[idxB], faceNormal);
		mesh->vnorms[idxC] = Add(mesh->vnorms[idxC], faceNormal);

	}

	// NORMALIZED EACH NORMAL ACCUMULATED
	for (int i = 0; i < nv; i++) {
		mesh->vnorms[i] = Normalize(mesh->vnorms[i]);
	}

	mesh->next = *list;
	*list = mesh;
}
