#include <iostream>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>

using namespace std;

class CylinderObject {
public:
	vector<float> getCircleVerts(int sectorCount) {
		const float PI = 3.1415926f;
		float sectorStep = 2 * PI / sectorCount;
		float sectorAngle;  // radian

		vector<float> unitCircleVertices;
		for (int i = 0; i <= sectorCount; ++i)
		{
			sectorAngle = i * sectorStep;
			unitCircleVertices.push_back(cos(sectorAngle)); // x
			unitCircleVertices.push_back(sin(sectorAngle)); // y
			unitCircleVertices.push_back(0);                // z
		}
		return unitCircleVertices;
	}
	vector<float> BuildVertsSmooth(float height, float radius, int sectorCount) {
		// clear memory of prev arrays
		std::vector<float> vertices;
		std::vector<float> normals;
		std::vector<float> texCoords;
		// get unit circle vectors on XY-plane
		std::vector<float> unitVertices = getCircleVerts(sectorCount);

		// put side vertices to arrays
		for (int i = 0; i < 2; ++i)
		{
			float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
			float t = 1.0f - i;                              // vertical tex coord; 1 to 0

			for (int j = 0, k = 0; j <= sectorCount; ++j, k += 3)
			{
				float ux = unitVertices[k];
				float uy = unitVertices[k + 1];
				float uz = unitVertices[k + 2];
				// position vector
				vertices.push_back(ux * radius);             // vx
				vertices.push_back(uy * radius);             // vy
				vertices.push_back(h);                       // vz
				// normal vector
				normals.push_back(ux);                       // nx
				normals.push_back(uy);                       // ny
				normals.push_back(uz);                       // nz
				// texture coordinate
				texCoords.push_back((float)j / sectorCount); // s
				texCoords.push_back(t);                      // t
			}
		}

		int baseCenterIndex = (int)vertices.size() / 3;
		int topCenterIndex = baseCenterIndex + sectorCount + 1; // include center vertex

		// put base and top vertices to arrays
		for (int i = 0; i < 2; ++i)
		{
			float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
			float nz = -1 + i * 2;                           // z value of normal; -1 to 1

			// center point
			vertices.push_back(0);     vertices.push_back(0);     vertices.push_back(h);
			normals.push_back(0);      normals.push_back(0);      normals.push_back(nz);
			texCoords.push_back(0.5f); texCoords.push_back(0.5f);

			for (int j = 0, k = 0; j < sectorCount; ++j, k += 3)
			{
				float ux = unitVertices[k];
				float uy = unitVertices[k + 1];
				// position vector
				vertices.push_back(ux * radius);             // vx
				vertices.push_back(uy * radius);             // vy
				vertices.push_back(h);                       // vz
				// normal vector
				normals.push_back(0);                        // nx
				normals.push_back(0);                        // ny
				normals.push_back(nz);                       // nz
				// texture coordinate
				texCoords.push_back(-ux * 0.5f + 0.5f);      // s
				texCoords.push_back(-uy * 0.5f + 0.5f);      // t
			}
		}

		// Construct the GLfloat array
		std::vector<float> model;
		for (int verts = 0, norms = 0, text = 0; verts < vertices.size(); verts += 3, norms += 3, text + 2) {
			// verts
			model.push_back(vertices.at(verts));
			model.push_back(vertices.at(verts + 1));
			model.push_back(vertices.at(verts + 2));
			// normal
			model.push_back(normals.at(norms));
			model.push_back(normals.at(norms + 1));
			model.push_back(normals.at(norms + 2));
			// text
			model.push_back(texCoords.at(text));
			model.push_back(texCoords.at(text + 1));

		}
		return model;
	}
	GLuint vao;
	GLuint vbo;
	GLuint nIndices;
	GLuint nVertices;
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;
	glm::mat4 model;
private:
	vector<float> verts;
	vector<float> indices;
};




