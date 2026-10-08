#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	//If you follow the setup instructions provided for Visual Studio
	//The include path specified already includes "GL", so we can ignore it
	#include <GL/glew.h>
	#include <GL/freeglut.h>
#elif defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
	#include <GL/glew.h>
	#include <GLUT/glut.h>
#else
	#include <GL/glew.h>
	#include <GL/freeglut.h>
#endif
#include "camera.h"	//algebra is included in camera.h
#include "shaders.h"
#include "mesh.h"

Camera cam = { { 5.0f, 7.0f, 20.0f }, { -13.0f, 13.0f, 0.0f }, 60, 1, 10000}; // Setup the global camera parameters

int screen_width = 1024;
int screen_height = 768;

Mesh *meshList = NULL; // Global pointer to linked list of triangle meshes

const int MAX_SHADERS = 10;
GLuint shprg[MAX_SHADERS]; // Shader program id
int activeShader = 0;

int windowHandle;

// GLOBAL VARIABLES ADDDED
const float PI = 3.14159265359f;
bool isPerspective = true; 
float knotRotationY = 0.0f;
bool rotation = true;

// Include data for some triangle meshes (hard coded in struct variables)
#include "./models/mesh_bunny.h"
#include "./models/mesh_cow.h"
#include "./models/mesh_cube.h"
#include "./models/mesh_frog.h"
#include "./models/mesh_knot.h"
#include "./models/mesh_sphere.h"
#include "./models/mesh_teapot.h"
#include "./models/mesh_triceratops.h"

// Global transform matrices
// V is the view transform
// P is the projection transform
// PV = P * V is the combined view-projection transform
Matrix V, P, PV;
void printShaderErrorInfo(GLuint shaderProgram)
{
	GLint maxLength = 0;
	glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

	// The maxLength includes the NULL character
	GLchar * errorLog = (GLchar*)calloc(maxLength, sizeof(GLchar));
	glGetShaderInfoLog(shaderProgram, maxLength, &maxLength, &errorLog[0]);

	// Provide the infolog in whatever manor you deem best.
	printf("%s\n", errorLog);
	printf("\n");
	free(errorLog);
}

char* readShaders(const char* fileName) {
	if (fileName == NULL) exit(-1);
	
	// OPEN FILE AND GET POINTER
	FILE* file = fopen(fileName, "rb");
	// CHECK POINTER
	if (file == NULL)
	{
		printf("File not found\n");
		exit(-2);
	}

	// GO TO THE END
	fseek(file, 0, SEEK_END);
	// TELL POSITION
	long size = ftell(file);
	if (size <= 0) {
		printf("Buffer size error\n");
		fclose(file);
		exit(-3);
	}
	// GO TO BEGGINING
	rewind(file);

	size_t sizet = (size_t)size;

	// CREATE BUFFER 
	char* buffer = (char*)malloc(size + 1);
	// CHECK BUFFER
	if (buffer == NULL) {
		printf("Buffer error\n");
		fclose(file);
		exit(-4);
	}

	// READ FILE
	size_t bytesRead = fread(buffer, 1, size, file);
	buffer[bytesRead] = '\0';

	// CLOSE FILE
	fclose(file);
	return buffer;
}

void update(int value) {
	
	if (rotation) {
		printf("Tiempo transcurrido\n");

		knotRotationY += 2.0f;
		if (knotRotationY > 360.0f) knotRotationY -= 360.0f;

		glutPostRedisplay();

		glutTimerFunc(20, update, 0);
	}
}

GLuint prepareShaderProgram(const char * vertName, const char * fragName) {
	GLint success = GL_FALSE;

	GLuint shprgLocal = glCreateProgram();

	char* vertCode = readShaders(vertName);
	char* fragCode = readShaders(fragName);

	printf("--- VERTEX SHADER CODE ---\n%s\n--------------------------\n", vertCode);
	printf("--- FRAGMENT SHADER CODE ---\n%s\n--------------------------\n", fragCode);

	GLuint vs;
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertCode, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success) printf("Error in vertex shader!\n");
	else printf("Vertex shader compiled successfully!\n");
	if(success == GL_FALSE)
	{
		printShaderErrorInfo(vs);
		glDeleteShader(vs); // Don't leak the shader.
	}

	GLuint fs;
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragCode, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success) printf("Error in fragment shader!\n");
	else printf("Fragment shader compiled successfully!\n");
	if(success == GL_FALSE)
	{
		printShaderErrorInfo(fs);
		glDeleteShader(fs); // Don't leak the shader.
	}

	glAttachShader(shprgLocal, vs);
	glAttachShader(shprgLocal, fs);

	// BINDS MEMORY POSITIONS IN GPU FOR NO DATA LOSS
	glBindAttribLocation(shprgLocal, 0, "vPos");
	glBindAttribLocation(shprgLocal, 1, "vNorm");

	glLinkProgram(shprgLocal);
	GLint isLinked = GL_FALSE;
	glGetProgramiv(shprgLocal, GL_LINK_STATUS, &isLinked);
	if (!isLinked) printf("Link error in shader program!\n");
	else printf("Shader program linked successfully!\n");

	return shprgLocal;
}

void prepareMesh(Mesh *mesh) {
	int sizeVerts = mesh->nv * 3 * sizeof(float);
	int sizeCols  = mesh->nv * 3 * sizeof(float);
	int sizeTris = mesh->nt * 3 * sizeof(int);
	
	// For storage of state and other buffer objects needed for vertex specification
	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	// Allocate VBO and load mesh data (vertices and normals)
	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeVerts + sizeCols, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVerts, (void *)mesh->vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeVerts, sizeCols, (void *)mesh->vnorms);

	// Allocate index buffer and load mesh indices
	glGenBuffers(1, &mesh->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeTris, (void *)mesh->triangles, GL_STATIC_DRAW);


	// Define the format of the vertex data
	GLint vPos = glGetAttribLocation(shprg[activeShader], "vPos");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Define the format of the vertex data 
	GLint vNorm = glGetAttribLocation(shprg[activeShader], "vNorm");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, (void *)(mesh->nv * 3 *sizeof(float)));

	glBindVertexArray(0);

}

// IMPORTANT NOTE: The renderMesh function is called for each mesh in the scene every time the window needs to be redrawn, for example when the user moves the camera.

void renderMesh(Mesh *mesh) {
	
	// Assignment TODO: Apply the transforms from local mesh coordinates to world coordinates here
	// Combine it with the viewing transform that is passed to the shader below
	
	Matrix T = TranslationMatrix(mesh->translation);
	Matrix S = ScalingMatrix(mesh->scaling);
	Matrix Rx = RotationMatrixX(mesh->rotation.x * (PI / 180.0f));
	Matrix Ry = RotationMatrixY(mesh->rotation.y * (PI / 180.0f));
	Matrix Rz = RotationMatrixZ(mesh->rotation.z * (PI / 180.0f));

	float currentRotY = mesh->rotation.y;

	// Si el objeto es el Knot, aplicamos la rotación animada
	if (mesh->nv == knot.nov) {
		currentRotY += knotRotationY; // Añadimos los 2 grados acumulados
	}

	Ry = RotationMatrixY(currentRotY * (PI / 180.0f));

	Matrix tmp1 = MatMatMul(Rz, S);       
	Matrix tmp2 = MatMatMul(Ry, tmp1);    
	Matrix tmp3 = MatMatMul(Rx, tmp2);    
	Matrix W = MatMatMul(T, tmp3);  

	GLuint program = shprg[activeShader];
	if (activeShader == 2 || activeShader == 3) {
		if (mesh->nv == cube.nov) { // Material Triceratops
			glUniform3f(glGetUniformLocation(program, "ambientColor"), 0.05f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(program, "diffuseColor"), 0.6f, 0.4f, 0.4f);
		}
		else if (mesh->nv == knot.nov) { // Material Cow
			glUniform3f(glGetUniformLocation(program, "ambientColor"), 0.329412f, 0.223529f, 0.027451f);
			glUniform3f(glGetUniformLocation(program, "diffuseColor"), 0.780392f, 0.568627f, 0.113725f);
		}
	}

	Matrix PVW = MatMatMul(PV, W);

	if (mesh == meshList) { // Just print for the first object to avoid spam
		PrintMatrix("W Matrix", W);
		PrintMatrix("V Matrix", V);
		PrintMatrix("PVW Matrix", PVW);
	}

	// Pass the viewing transform to the shader
	GLint loc_PV = glGetUniformLocation(shprg[activeShader], "PV");
	glUniformMatrix4fv(loc_PV, 1, GL_FALSE, PV.e);
	GLint loc_W = glGetUniformLocation(shprg[activeShader], "W");
	glUniformMatrix4fv(loc_W, 1, GL_FALSE, W.e);

	// Solo enviamos estos datos si el shader activo es el de Phong (el número 2)
	if (activeShader == 2) {
		GLuint program = shprg[activeShader];

		glUniform3f(glGetUniformLocation(program, "viewPos"), cam.position.x, cam.position.y, cam.position.z);

		// --- 1. DATOS DE LA LUZ (Iguales para todos los objetos) --- 
		glUniform3f(glGetUniformLocation(program, "lightPosition"), -20.0f, 10.0f, 10.0f); 
		glUniform3f(glGetUniformLocation(program, "ambientLight"), 0.2f, 0.2f, 0.2f); 
		glUniform3f(glGetUniformLocation(program, "diffuseLight"), 0.9f, 0.9f, 0.7f); 
		glUniform3f(glGetUniformLocation(program, "specularLight"), 1.0f, 1.0f, 1.0f);
		
		// --- 2. DATOS DEL MATERIAL (Cambian según el objeto) ---
		// Si la malla que estamos dibujando es la Vaca
		if (mesh->nv == cow.nov) {
			glUniform3f(glGetUniformLocation(program, "ambientColor"), 0.329412f, 0.223529f, 0.027451f); 
			glUniform3f(glGetUniformLocation(program, "diffuseColor"), 0.780392f, 0.568627f, 0.113725f); 
			glUniform3f(glGetUniformLocation(program, "specularColor"), 0.992157f, 0.941176f, 0.807843f); 
			glUniform1f(glGetUniformLocation(program, "shininess"), 27.9f); 
		}
		// Si la malla es el Triceratops
		else if (mesh->nv == triceratops.nov) {
			glUniform3f(glGetUniformLocation(program, "ambientColor"), 0.05f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(program, "diffuseColor"), 0.6f, 0.4f, 0.4f); 
			glUniform3f(glGetUniformLocation(program, "specularColor"), 0.7f, 0.04f, 0.04f); 
			glUniform1f(glGetUniformLocation(program, "shininess"), 10.0f); 
		}
		// Si la malla es la Tetera
		else if (mesh->nv == teapot.nov) {
			glUniform3f(glGetUniformLocation(program, "ambientColor"), 0.19225f, 0.19225f, 0.19225f); 
			glUniform3f(glGetUniformLocation(program, "diffuseColor"), 0.50754f, 0.50754f, 0.50754f); 
			glUniform3f(glGetUniformLocation(program, "specularColor"), 0.508273f, 0.508273f, 0.508273f); 
			glUniform1f(glGetUniformLocation(program, "shininess"), 51.2f); 
		}
	}
	else if (activeShader == 3) {
		GLuint program = shprg[activeShader];

		glUniform3f(glGetUniformLocation(program, "lightPosition"), -20.0f, 10.0f, 10.0f);
		glUniform3f(glGetUniformLocation(program, "ambientLight"), 0.2f, 0.2f, 0.2f);
		glUniform3f(glGetUniformLocation(program, "diffuseLight"), 0.9f, 0.9f, 0.7f);

		// Si la malla que estamos dibujando es la Vaca
		if (mesh->nv == cow.nov) {
			glUniform3f(glGetUniformLocation(program, "ambientColor"), 0.329412f, 0.223529f, 0.027451f);
			glUniform3f(glGetUniformLocation(program, "diffuseColor"), 0.780392f, 0.568627f, 0.113725f);
		}
		// Si la malla es el Triceratops
		else if (mesh->nv == triceratops.nov) {
			glUniform3f(glGetUniformLocation(program, "ambientColor"), 0.05f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(program, "diffuseColor"), 0.6f, 0.4f, 0.4f);
		}
		// Si la malla es la Tetera
		else if (mesh->nv == teapot.nov) {
			glUniform3f(glGetUniformLocation(program, "ambientColor"), 0.19225f, 0.19225f, 0.19225f);
			glUniform3f(glGetUniformLocation(program, "diffuseColor"), 0.50754f, 0.50754f, 0.50754f);
		}
	}

	// Select current resources 
	glBindVertexArray(mesh->vao);
	
	// To accomplish wireframe rendering (can be removed to get filled triangles)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
	glEnable(GL_DEPTH_TEST);

	// Draw all triangles
	glDrawElements(GL_TRIANGLES, mesh->nt * 3, GL_UNSIGNED_INT, NULL); 
	glBindVertexArray(0);
}

// IMPORTANT NOTE: The display function is called every time the window needs to be redrawn, for example when the user moves the camera.

void display(void) {
	Mesh *mesh;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
	Assignment TODO: The implementation of the View- and ProjectionMatrix functions in camera.cpp 
	returns hardcoded matrices. Your assignment is to fix them.
	*/
	V = ViewMatrix(cam.position, cam.rotation);
	
	if (isPerspective) {
		P = PerspectiveProjectionMatrix(cam, screen_width, screen_height);
	}
	else {
		P = OrthographicMatrix(-20.0f, 20.0f, -10.0f, 10.0f, 1.0f, 1000.0f);
	}

	// This finds the combined view-projection matrix
	PV = MatMatMul(P, V);

	// Select the shader program to be used during rendering 
	glUseProgram(shprg[activeShader]);

	// Render all meshes in the scene
	mesh = meshList;
		
	while (mesh != NULL) {
		renderMesh(mesh);
		mesh = mesh->next;
	}

	glFlush();
}

void changeSize(int w, int h) {
	screen_width = w;
	screen_height = h;
	glViewport(0, 0, screen_width, screen_height);

}
void cleanUp(void);
void keypress(unsigned char key, int x, int y) {
	switch (key) {
	case '1':
		activeShader = 0; // Cambia al shader básico (arcoíris)
		printf("Usando Shader: Básico\n");
		break;
	case '2':
		activeShader = 1; // Cambia al shader entrelazado
		printf("Usando Shader: Entrelazado\n");
		break;
	case '3':
		activeShader = 2; // Cambia al shader Phong
		printf("Usando Shader: Phong\n");
		break;
	case '4':
		activeShader = 3; // Cambia al shader Phong
		printf("Usando Shader: Cartoon\n");
		break;
	case 'z':
		cam.position.z -= 0.2f;
		break;
	case 'p':
		rotation = !rotation;
		break;
	case 'P':
		isPerspective = !isPerspective;
		if (isPerspective) printf("Proyección: Perspectiva\n");
		else printf("Proyección: Ortogonal\n");
		break;

	case 'f':
		cam.fov -= 2.0f; 
		printf("FOV: %f\n", cam.fov);
		break;
	case 'F':
		cam.fov += 2.0f; 
		printf("FOV: %f\n", cam.fov);
		break;
	case 'Z':
		cam.position.z += 0.2f;
		break;

	case 'x':
		cam.position.x -= 0.2f;
		break;
	case 'X':
		cam.position.x += 0.2f;
		break;

	case 'y':
		cam.position.y -= 0.2f;
		break;
	case 'Y':
		cam.position.y += 0.2f;
		break;

	case 'i':
		cam.rotation.x -= 2.0f;
		break;
	case 'I':
		cam.rotation.x += 2.0f;
		break;

	case 'j':
		cam.rotation.y -= 2.0f;
		break;
	case 'J':
		cam.rotation.y += 2.0f;
		break;

	case 'k':
		cam.rotation.z -= 2.0f;
		break;
	case 'K':
		cam.rotation.z += 2.0f;
		break;

	case 'Q':
	case 'q':
#ifdef __APPLE__
		// MAC OS GLUT does not implement LeaveMainLoop().
		cleanUp();
		exit(0);
#else
		glutLeaveMainLoop();
#endif
		break;
	}

	glutPostRedisplay();
}

void init(void) {
	// Compile and link the given shader program (vertex shader and fragment shader)
	shprg[0] = prepareShaderProgram("shaders/simple.vert.txt", "shaders/simple.frag.txt");
	shprg[1] = prepareShaderProgram("InterlacedVertexShader.glsl", "InterlacedFragShader.glsl");
	shprg[2] = prepareShaderProgram("shaders/phong.vert.glsl", "shaders/phong.frag.glsl");
	shprg[3] = prepareShaderProgram("shaders/cartoon.vert.glsl", "shaders/cartoon.frag.glsl");


	// Setup OpenGL buffers for rendering of the meshes
	Mesh * mesh = meshList;
	while (mesh != NULL) {
		prepareMesh(mesh);
		mesh = mesh->next;
	}	
}

void cleanUp(void) {	
	printf("Running cleanUp function... ");
	// Free openGL resources
	// ...
	glutDestroyWindow(windowHandle);

	// Free meshes
	// ...
	
	printf("Done!\n\n");
}



int main(int argc, char **argv) {
	
	// Setup freeGLUT
	unsigned int _glut_mode = GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH;
	#ifdef __APPLE__
	_glut_mode |= GLUT_3_2_CORE_PROFILE;
	#endif
	glutInitDisplayMode(_glut_mode);
	glutInit(&argc, argv);
	glutInitWindowSize(screen_width, screen_height);
	windowHandle = glutCreateWindow("DVA338 Programming Assignments");
	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keypress);
	//This is a freeglut define value. In MacOS GLUT, these functions do not exist.
	#ifdef GLUT_CORE_PROFILE 
  	glutInitContextVersion(4, 1);
  	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	#endif
	
	glewExperimental = GL_TRUE;
	// Uses GLEW as OpenGL Loading Library to get access to modern core features as well as extensions
	GLenum err = glewInit(); 
	if (GLEW_OK != err) { fprintf(stdout, "Error: %s\n", glewGetErrorString(err)); return 1; }

	// Output OpenGL version info
	fprintf(stdout, "GLEW version: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, "OpenGL version: %s\n", (const char *)glGetString(GL_VERSION));
	fprintf(stdout, "OpenGL vendor: %s\n\n", glGetString(GL_VENDOR));


	// Insert the 3D models you want in your scene here in a linked list of meshes
	// Note that "meshList" is a pointer to the first mesh and new meshes are added to the front of the list	
	/*Vector translationCow = {0.0f, 1.5f, -8.0f};
	Vector rotationCow = { 0.0f, -90.0f, 0.0f };
	Vector scaleCow = { 20.0f, 20.0f, 20.0f }; // ¡Escala real, no ceros!
	insertModel(&meshList, cow.nov, cow.verts, cow.nof, cow.faces, translationCow, scaleCow, rotationCow);

	Vector transTri = { -6.8f, 0.8f, 5.0f };
	Vector rotTri = { 0.0f, 0.0f, 0.0f };
	Vector scaleTri = { 3.0f, 3.0f, 3.0f };
	scaleTri = ScalarVecMul(0.1, scaleTri);
	insertModel(&meshList, triceratops.nov, triceratops.verts, triceratops.nof, triceratops.faces, transTri, scaleTri, rotTri);
	*/
	//insertModel(&meshList, bunny.nov, bunny.verts, bunny.nof, bunny.faces, 60.0);	
	Vector transCube = { -10.0f, -1.0f, -10.0f };
	Vector scaleCube = { 4.0f, 0.1f, 4.0f };
	insertModel(&meshList, cube.nov, cube.verts, cube.nof, cube.faces, transCube, scaleCube, {0,0,0});
	//insertModel(&meshList, frog.nov, frog.verts, frog.nof, frog.faces, 2.5);
	Vector transKnot = { 0.0f, 4.0f, 0.0f };
	Vector scaleKnot = { 0.4f, 0.4f, 0.4f };
	insertModel(&meshList, knot.nov, knot.verts, knot.nof, knot.faces, transKnot, scaleKnot, {0,0,0});
	//insertModel(&meshList, sphere.nov, sphere.verts, sphere.nof, sphere.faces, 12.0);
	/*Vector transTea = {0.0f, -7.0f, 0.0f};
	Vector scaleTea = { 3.0f, 3.0f, 3.0f };
	Vector rotTea = { -90.0f, 0.0f, 180.0f };
	insertModel(&meshList, teapot.nov, teapot.verts, teapot.nof, teapot.faces, transTea, scaleTea, rotTea);
	*/

	init();
	glutTimerFunc(16, update, 0); // Inicia el ciclo 
	glutMainLoop();

	cleanUp();	
	return 0;
}
