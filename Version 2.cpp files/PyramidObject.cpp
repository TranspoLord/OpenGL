#include <iostream>
#include <glm/glm.hpp>
#include <GL/glew.h>

using namespace std;

class PyramidObject {
public:
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;
	glm::mat4 model;

	GLuint vao;
	GLuint vbo[2];
	GLuint nIndices;

	GLfloat verts[40] = {
		// Vertex Positions    //Norms				//Texture Coords
		-1.0f,  0.0f, -1.0f,   0.0f, 0.0f, 1.0f,	1.0f, 1.0f, // Bottom right front 0
		 1.0f,  0.0f, -1.0f,   0.0f, 1.0f, 1.0f,	1.0f, 0.0f, // Bottom right back 1
		 1.0f,  0.0f,  1.0f,   1.0f, 1.0f, 0.0f,	0.0f, 0.0f, // Bottom left front 2
		-1.0f,  0.0f,  1.0f,   1.0f, 1.0f, 1.0f,	0.0f, 1.0f, // Bottom left back 3
		 0.0f,  2.0f,  0.0f,   1.0f, 0.0f, 0.0f,	0.5f, 0.5f, // Top 4
	};

	GLushort indices[18] = {
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 1, 4,
			2, 1, 4,
			2, 3, 4
	};
};