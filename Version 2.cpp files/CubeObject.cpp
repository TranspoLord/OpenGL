#include <iostream>
#include <glm/glm.hpp>
#include <GL/glew.h>

using namespace std;

class CubeObject {
public:
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;
	glm::mat4 model;

	GLuint vao;
	GLuint vbo[2];
	GLuint nIndices;

	GLshort indices[36] = {
				0, 1, 3, // Triangle 1
				1, 2, 3, // Triangle 2
				0, 1, 4, // Triangle 3
				0, 4, 5, // Triangle 4
				0, 5, 6, // Triangle 5
				0, 3, 6, // Triangle 6
				4, 5, 6, // Triangle 7
				4, 6, 7, // Triangle 8
				2, 3, 6, // Triangle 9
				2, 6, 7, // Triangle 10
				1, 4, 7, // Triangle 11
				1, 2, 7, // Triangle 12
	};
	GLfloat verts[68] = {
		// Vertex Positions    //Normals		//Texture Coords
		 0.5f,   0.5f,  0.0f,  0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // 0
		 0.5f,  -0.5f,  0.0f,  0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // 1
		-0.5f,  -0.5f,  0.0f,  0.0f, 0.0f,  1.0f,   1.0f, 1.0f, // 2
		-0.5f,   0.5f,  0.0f,  0.0f, 0.0f,  1.0f,	0.0f, 0.0f, // 3

		 0.5f,  -0.5f, -1.0f,  -1.0f, 0.0f,  0.0f,	1.0f, 1.0f, // 4
		 0.5f,   0.5f, -1.0f,  -1.0f, 0.0f,  0.0f,	0.0f, 1.0f, // 5
		-0.5f,   0.5f, -1.0f,  1.0f, 0.0f,  0.0f,	1.0f, 0.0f, // 6
		-0.5f,  -0.5f, -1.0f,  1.0f, 0.0f,  0.0f,	0.0f, 1.0f  // 7
	};
};