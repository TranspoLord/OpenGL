#include <iostream>       
#include <cstdlib>     
#include <GL/glew.h>        
#include <GLFW/glfw3.h>    
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnOpengl/camera.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "CubeObject.cpp"
#include "PyramidObject.cpp"
#include "CylinderObject.cpp"
#include <vector>

using namespace std;
float counter = 0.0f;

#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

//Unnamed namespace
namespace {
	const char* const Window_Title = "Texturing";
	const int Window_Height = 1024;
	const int Window_Width = 1280;

	GLFWwindow* mainWindow = nullptr;
	GLuint mainProgramID;
	GLuint lightProgramID;

	glm::vec3 lightColor[2] = { glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f) };
	glm::vec3 lightPos[2] = { glm::vec3(0.0f, -2.0f, 3.85f), glm::vec3(-0.10f, 3.85f, -0.5f) };
	glm::vec3 lightScale(0.0f);

	//Just a struct to hold textures. Grouping to make it easier Ex: textures.brick
	struct Texture {
		GLuint brick;
		GLuint blackFabric;
		GLuint navyCarpet;
		GLuint greyPlastic;
		GLuint blueFabric;
		GLuint woodenLegs;
		GLuint greyMetal;
		GLuint screen;
		GLuint glass;
		GLuint blackPlastic;
	};
	Texture textures;

	CubeObject tableTop, frontRightLeg, frontLeftLeg, backRightLeg, backLeftLeg, plane;
	CubeObject couchFRLeg, couchFLLeg, couchBRLeg, couchBLLeg, couchSeatSupport, couchLeftCushion, couchRightCushion, couchBackSupport, couchRightArm, couchLeftArm;
	CubeObject laptopBase, laptopScreen, laptopBack;
	CubeObject longCouchBase, longCouchFRLeg, longCouchBRLeg, longCouchFLLeg, longCouchBLLeg, longCouchCushion1, longCouchCushion2, longCouchCushion3, longCouchCushion4;
	CubeObject longCouchLeftArm, longCouchRightArm, longCouchBack;
	CubeObject smallTableTop, smallTableMid, smallTableFRLeg, smallTableFLLeg, smallTableBRLeg, smallTableBLLeg;
	CubeObject cabinetBase, cabinetLeftSide, cabinetRightSide, cabinetBack, cabinetTop, cabinetShelf1, cabinetShelf2, cabinetShelf3, cabinetInnerWall1, cabinetInnerWall2;
	PyramidObject decor1, decor2, decor3;

	CubeObject textureCube;
	CubeObject lightCube1, lightCube2;

	CylinderObject testCyl;
}

//Camera variables
Camera mainCamera(glm::vec3(0.0f, 0.0f, 3.0f));
struct cameraVars {
	float mainPrevX = Window_Width / 2.0f;
	float mainPrevZ = Window_Height / 2.0f;
	bool firstMouse = true;
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float camSpeed = 2.5f;
	bool changeView = false;
	bool speedChange = false;
};
cameraVars camVars;

//User created functions
bool Initialize(int argc, char* argv[], GLFWwindow** window);
void WindowResize(GLFWwindow* window, int width, int height);
void ProcessInput(GLFWwindow* window);
void MeshCreation(CubeObject& objMesh);
void MeshDeletion(CubeObject& objMesh);
void RenderFrame();
bool CreateShaders(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programID);
void DestroyShaders(GLuint programID);
void DrawObject(CubeObject obj, GLuint texture, GLint viewPosLoc, glm::mat4 view, glm::mat4 projection);
void MousePositon(GLFWwindow* window, double xPos, double zPos);
void MouseScroll(GLFWwindow* window, double xSet, double zSet);
void MouseButton(GLFWwindow* window, int button, int action, int mods);
void ObjectScaleTransRotate(float sX, float sY, float sZ, float tX, float tY, float tZ, float rX, float rY, float rZ, float rMulti, CubeObject& obj);
void MainMeshCreation();
void MainMeshDeletion();
void MainTextureCreation();
void MainTextureDeletion();
void TextureCreation(const char* fileName, GLuint& texture);
void TextureDeletion(GLuint texture);
void ImageFlipVert(unsigned char* image, int width, int height, int channels);
void DrawLightObject(CubeObject& obj, glm::mat4 view, glm::mat4 projection, GLint lightColorLoc, GLint lightPosLoc, int index);
void MainObjectScaleTransRotate();
void MainDrawObject(GLint viewPosLoc, glm::mat4 view, glm::mat4 projection);

void MeshCreation(PyramidObject& objMesh);
void MeshDeletion(PyramidObject& objMesh);
void ObjectScaleTransRotate(float sX, float sY, float sZ, float tX, float tY, float tZ, float rX, float rY, float rZ, float rMulti, PyramidObject& obj);
void DrawObject(PyramidObject obj, GLuint texture, GLint viewPosLoc, glm::mat4 view, glm::mat4 projection);

void MeshCreation(CylinderObject& objMesh);
void MeshDeletion(CylinderObject& objMesh);
void DrawObject(CylinderObject obj, GLuint texture, GLint viewPosLoc, glm::mat4 view, glm::mat4 projection);

//Vertex Shader
const GLchar* objectVertexShader = GLSL(440,
	layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textureCoordinate;

out vec2 vertexTextureCoordinate;
out vec3 vertexFragPos;
out vec3 vertexNorm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
	vertexFragPos = vec3(model * vec4(position, 1.0f));
	vertexNorm = mat3(transpose(inverse(model))) * normal;
	vertexTextureCoordinate = textureCoordinate;
}
);

//Fragment shader
const GLchar* objectFragmentShader = GLSL(440,
	in vec3 vertexNorm;
in vec3 vertexFragPos;
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

uniform vec3 lightColor[2] = vec3[](vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.1f));
uniform vec3 lightPos[2] = vec3[](vec3(-0.10f, 5.85f, -0.5f), vec3(-0.10f, 0.85f, -0.5f));

uniform vec3 viewPosition;
uniform sampler2D textureBase;

struct lightDetails {
	float ambientStrength;
	float specIntens;
	float highlightSize;

	vec3 ambient;
	vec3 diffuse;
	vec3 spec;
	vec3 phong;
};

lightDetails light[2];

void main()
{
	light[0].ambientStrength = 0.5f;
	light[1].ambientStrength = 0.00f;

	light[0].specIntens = 1.0f;
	light[1].specIntens = 0.0f;

	light[0].highlightSize = 1.0f;
	light[1].highlightSize = 0.0f;

	vec3 totalPhong;
	for (int i = 0; i < 2; i++) {
		//Ambient
		light[i].ambient = light[i].ambientStrength * lightColor[i];
		//Diffuse
		vec3 norm = normalize(vertexNorm);
		vec3 lightDirection = normalize(lightPos[i] - vertexFragPos);
		float impact = max(dot(norm, lightDirection), 0.0);
		light[i].diffuse = impact * lightColor[i];
		//Spec
		vec3 viewDir = normalize(viewPosition - vertexFragPos);
		vec3 reflectDir = reflect(-lightDirection, norm);
		float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), light[i].highlightSize);
		vec3 specular = light[i].specIntens * specularComponent * lightColor[i];
		vec4 textureColor = texture(textureBase, vertexTextureCoordinate);
		light[i].phong = (light[i].ambient + light[i].diffuse + light[i].spec) * textureColor.xyz;

		totalPhong = totalPhong + light[i].phong;
	}

	fragmentColor = vec4(totalPhong, 1.0);
}
);

//Light vertex shader 
const GLchar* lightVertexShader = GLSL(440,
	layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
}
);

//Light frag shader
const GLchar* lightFragmentShader = GLSL(440,
	out vec4 fragmentColor;

void main()
{
	fragmentColor = vec4(1.0f);
}
);

int main(int argc, char* argv[]) {
	//Sets window, mesh and shaders
	if (!Initialize(argc, argv, &mainWindow))
		return EXIT_FAILURE;
	MainMeshCreation();

	if (!CreateShaders(objectVertexShader, objectFragmentShader, mainProgramID)) {
		cout << "Object shader failure" << endl;
		return EXIT_FAILURE;
	}
	if (!CreateShaders(lightVertexShader, lightFragmentShader, lightProgramID)) {
		cout << "Lighting shader failure" << endl;
		return EXIT_FAILURE;
	}

	MainTextureCreation();
	glUseProgram(mainProgramID);
	glUniform1i(glGetUniformLocation(mainProgramID, "textureBase"), 0);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	//Rendering Loop
	while (!glfwWindowShouldClose(mainWindow)) {

		//Gets current time
		float curFrame = glfwGetTime();
		camVars.deltaTime = curFrame - camVars.lastFrame;
		camVars.lastFrame = curFrame;

		ProcessInput(mainWindow);
		RenderFrame();
		glfwPollEvents();
	}

	MainMeshDeletion();
	MainTextureDeletion();
	DestroyShaders(mainProgramID);
	DestroyShaders(lightProgramID);
	exit(EXIT_SUCCESS);
}


//Creates and binds mesh
void MeshCreation(CubeObject& obj) {
	const GLuint fPerVer = 3;
	const GLuint fPerNorm = 3;
	const GLuint fPerText = 2;

	cout << "Object indices size: " << sizeof(obj.indices) << endl;
	cout << "Object vertices size: " << sizeof(obj.verts) << endl;

	obj.nIndices = sizeof(obj.indices) / sizeof(obj.indices[0]);
	glGenVertexArrays(1, &obj.vao);
	glBindVertexArray(obj.vao);

	glGenBuffers(2, obj.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(obj.verts), obj.verts, GL_STATIC_DRAW);

	GLint stride = sizeof(float) * (fPerVer + fPerText + fPerNorm);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(obj.indices), obj.indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, fPerVer, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, fPerNorm, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * fPerVer));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, fPerText, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (fPerVer + fPerNorm)));
	glEnableVertexAttribArray(2);
}
void MeshCreation(PyramidObject& obj) {
	const GLuint fPerVer = 3;
	const GLuint fPerNorm = 3;
	const GLuint fPerText = 2;

	cout << "Object indices size: " << sizeof(obj.indices) << endl;
	cout << "Object vertices size: " << sizeof(obj.verts) << endl;

	obj.nIndices = sizeof(obj.indices) / sizeof(obj.indices[0]);
	glGenVertexArrays(1, &obj.vao);
	glBindVertexArray(obj.vao);

	glGenBuffers(2, obj.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(obj.verts), obj.verts, GL_STATIC_DRAW);

	GLint stride = sizeof(float) * (fPerVer + fPerText + fPerNorm);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(obj.indices), obj.indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, fPerVer, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, fPerNorm, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * fPerVer));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, fPerText, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (fPerVer + fPerNorm)));
	glEnableVertexAttribArray(2);
}
void MeshCreation(CylinderObject& obj) {
	std::vector<float> verts = obj.BuildVertsSmooth(2.0f, 2.0f, 24);


	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;

	// mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
	obj.nVertices = (verts.size() * sizeof(GLfloat)) / (sizeof(GLfloat) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

	glGenVertexArrays(1, &obj.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(obj.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(1, &obj.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), &verts[0], GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(GLfloat) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * floatsPerVertex));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}

//Code needed for creating a frame
void RenderFrame() {
	//Endables depth rendering
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Object scale, trans, rotate call. Calls a function that sets the objects placement
	MainObjectScaleTransRotate();

	//set camera view before checking 
	glm::mat4 projection = glm::perspective(glm::radians(mainCamera.Zoom), (GLfloat)Window_Width / (GLfloat)Window_Height, 0.1f, 100.0f);

	//Changes view of the camera if the users presses the respective button
	glm::mat4 view = mainCamera.GetViewMatrix();
	if (!camVars.changeView)
		projection = glm::perspective(glm::radians(mainCamera.Zoom), (GLfloat)Window_Width / (GLfloat)Window_Height, 0.1f, 100.0f);
	if (camVars.changeView)
		projection = glm::ortho(glm::radians(mainCamera.Zoom), (GLfloat)Window_Width / (GLfloat)Window_Height, 0.1f, 100.0f);

	glUseProgram(mainProgramID);
	GLint modelLoc = glGetUniformLocation(mainProgramID, "model");
	GLint viewLoc = glGetUniformLocation(mainProgramID, "view");
	GLint projLoc = glGetUniformLocation(mainProgramID, "projection");
	GLint objectColorLoc = glGetUniformLocation(mainProgramID, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(mainProgramID, "lightColor");
	GLint lightPositionLoc = glGetUniformLocation(mainProgramID, "lightPos");
	GLint viewPositionLoc = glGetUniformLocation(mainProgramID, "viewPosition");

	//Sends data to a function where the DrawObject func is called many times
	//sent there to keep large blocks of code out, make it more clean
	MainDrawObject(viewPositionLoc, view, projection);

	glUseProgram(lightProgramID);

	//Deactivates the VAO and shaders
	glBindVertexArray(0);
	glfwSwapBuffers(mainWindow);
}

//Simple func to reduce lines of code. Changes the scale, translation, and rotation of an object with the float passed into it
void ObjectScaleTransRotate(float sX, float sY, float sZ, float tX, float tY, float tZ, float rX, float rY, float rZ, float rMulti, CubeObject& obj) {
	obj.scale = glm::scale(glm::vec3(sX, sY, sZ));
	obj.translation = glm::translate(glm::vec3(tX, tY, tZ));
	obj.rotation = glm::rotate(rMulti, glm::vec3(rX, rY, rZ));
}
void ObjectScaleTransRotate(float sX, float sY, float sZ, float tX, float tY, float tZ, float rX, float rY, float rZ, float rMulti, PyramidObject& obj) {
	obj.scale = glm::scale(glm::vec3(sX, sY, sZ));
	obj.translation = glm::translate(glm::vec3(tX, tY, tZ));
	obj.rotation = glm::rotate(rMulti, glm::vec3(rX, rY, rZ));
}
void ObjectScaleTransRotate(float sX, float sY, float sZ, float tX, float tY, float tZ, float rX, float rY, float rZ, float rMulti, CylinderObject& obj) {
	obj.scale = glm::scale(glm::vec3(sX, sY, sZ));
	obj.translation = glm::translate(glm::vec3(tX, tY, tZ));
	obj.rotation = glm::rotate(rMulti, glm::vec3(rX, rY, rZ));
}

//User create function to draw the object given
void DrawObject(CubeObject obj, GLuint texture, GLint viewPosLoc, glm::mat4 view, glm::mat4 projection) {
	obj.model = obj.translation * obj.rotation * obj.scale;
	GLuint modelLoc = glGetUniformLocation(mainProgramID, "model");
	GLuint viewLoc = glGetUniformLocation(mainProgramID, "view");
	GLuint projLoc = glGetUniformLocation(mainProgramID, "projection");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	const glm::vec3 cameraPos = mainCamera.Position;
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	glBindVertexArray(obj.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawElements(GL_TRIANGLES, obj.nIndices, GL_UNSIGNED_SHORT, NULL);
}
void DrawObject(PyramidObject obj, GLuint texture, GLint viewPosLoc, glm::mat4 view, glm::mat4 projection) {
	obj.model = obj.translation * obj.rotation * obj.scale;
	GLuint modelLoc = glGetUniformLocation(mainProgramID, "model");
	GLuint viewLoc = glGetUniformLocation(mainProgramID, "view");
	GLuint projLoc = glGetUniformLocation(mainProgramID, "projection");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	const glm::vec3 cameraPos = mainCamera.Position;
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	glBindVertexArray(obj.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawElements(GL_TRIANGLES, obj.nIndices, GL_UNSIGNED_SHORT, NULL);
}
void DrawObject(CylinderObject obj, GLuint texture, GLint viewPosLoc, glm::mat4 view, glm::mat4 projection) {
	obj.model = obj.translation * obj.rotation * obj.scale;
	GLuint modelLoc = glGetUniformLocation(mainProgramID, "model");
	GLuint viewLoc = glGetUniformLocation(mainProgramID, "view");
	GLuint projLoc = glGetUniformLocation(mainProgramID, "projection");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	const glm::vec3 cameraPos = mainCamera.Position;
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	glBindVertexArray(obj.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);
}
void DrawLightObject(CubeObject& obj, glm::mat4 view, glm::mat4 projection, GLint lightColorLoc, GLint lightPosLoc, int index) {
	glm::mat4 model = obj.scale * obj.translation * obj.rotation;
	GLuint modelLoc = glGetUniformLocation(lightProgramID, "model");
	GLuint viewLoc = glGetUniformLocation(lightProgramID, "view");
	GLuint projLoc = glGetUniformLocation(lightProgramID, "projection");
	glUniform3f(lightColorLoc, lightColor[index].r, lightColor[index].g, lightColor[index].b);
	glUniform3f(lightPosLoc, lightPos[index].x, lightPos[index].y, lightPos[index].z);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawElements(GL_TRIANGLES, obj.nIndices, GL_UNSIGNED_SHORT, NULL);
}

//Loading a texture
void TextureCreation(const char* fileName, GLuint& texture) {
	int width, height, channels;
	bool loaded = false;
	unsigned char* image = stbi_load(fileName, &width, &height, &channels, 0);
	if (image) {
		ImageFlipVert(image, width, height, channels);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		//Texture wrap params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//Texture filter params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " numbers of channels" << endl;
			loaded = false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0);
		loaded = true;
	}
	if (!loaded) {
		cout << "Failed to load texture with path: " << fileName << endl;
		EXIT_FAILURE;
	}
}

//Deletes a texture
void TextureDeletion(GLuint texture) {
	glGenTextures(1, &texture);
}

//Initializes the window and program
bool Initialize(int argc, char* argv[], GLFWwindow** window) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	*window = glfwCreateWindow(Window_Width, Window_Height, Window_Title, NULL, NULL);
	if (*window == NULL) {
		cout << "Failed to create Window" << endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, WindowResize);
	glfwSetCursorPosCallback(*window, MousePositon);
	glfwSetScrollCallback(*window, MouseScroll);
	glfwSetMouseButtonCallback(*window, MouseButton);
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult) {
		cerr << glewGetErrorString(GlewInitResult) << endl;
		return false;
	}

	return true;
}

//Recalcs render if window is resized
void WindowResize(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

//Processes any input from any input device supported
void ProcessInput(GLFWwindow* window) {

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(FORWARD, camVars.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(BACKWARD, camVars.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(LEFT, camVars.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(RIGHT, camVars.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		camVars.changeView = !camVars.changeView;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		mainCamera.Position.y += camVars.camSpeed * camVars.deltaTime;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		mainCamera.Position.y -= camVars.camSpeed * camVars.deltaTime;
}

//Deletes mesh during program exit
void MeshDeletion(CubeObject& obj) {
	glDeleteVertexArrays(1, &obj.vao);
	glDeleteBuffers(2, obj.vbo);
}
void MeshDeletion(PyramidObject& obj) {
	glDeleteVertexArrays(1, &obj.vao);
	glDeleteBuffers(2, obj.vbo);
}
void MeshDeletion(CylinderObject& obj) {
	glDeleteVertexArrays(1, &obj.vao);
	glDeleteBuffers(2, &obj.vbo);
}

//Creates the shaders and links them to the program ID
bool CreateShaders(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programID) {
	int success = 0;
	char infoLog[512];

	programID = glCreateProgram();

	GLuint verShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fraShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(verShaderID, 1, &vtxShaderSource, NULL);
	glShaderSource(fraShaderID, 1, &fragShaderSource, NULL);

	glCompileShader(verShaderID);
	glGetShaderiv(verShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(verShaderID, sizeof(infoLog), NULL, infoLog);
		cout << "Vertex shader compilation failed\n" << infoLog << endl;
		return false;
	}

	glCompileShader(fraShaderID);
	glGetShaderiv(fraShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(verShaderID, sizeof(infoLog), NULL, infoLog);
		cout << "Frag shader compilation failed\n" << infoLog << endl;
		return false;
	}

	glAttachShader(programID, verShaderID);
	glAttachShader(programID, fraShaderID);

	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programID, sizeof(infoLog), NULL, infoLog);
		cout << "Linking shader to program failed\n" << infoLog << endl;
		return false;
	}

	glUseProgram(programID);
	return true;
}

//Destroys shaders on program exit
void DestroyShaders(GLuint programID) {
	glDeleteProgram(programID);
}

//Gets mouse position and uses it for camera control
void MousePositon(GLFWwindow* window, double xPos, double zPos) {
	if (camVars.firstMouse) {
		camVars.mainPrevX = xPos;
		camVars.mainPrevZ = zPos;
		camVars.firstMouse = false;
	}

	float xOffSet = xPos - camVars.mainPrevX;
	float zOffSet = zPos - camVars.mainPrevZ;
	camVars.mainPrevX = xPos;
	camVars.mainPrevZ = zPos;

	mainCamera.ProcessMouseMovement(xOffSet, zOffSet);
}

//Gets mouse scroll and changes cam speed
void MouseScroll(GLFWwindow* window, double xSet, double zSet) {
	mainCamera.ProcessMouseScroll(zSet);
}

//Provides feedback for button press on mouse doesn't actually do anything as of yet
void MouseButton(GLFWwindow* window, int button, int action, int mods) {
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT: {
		if (action == GLFW_PRESS)
			cout << "Left mouse pressed" << endl;
		else
			cout << "Left mouse released" << endl;
	}
							   break;

	case GLFW_MOUSE_BUTTON_RIGHT: {
		if (action == GLFW_PRESS)
			cout << "Right mouse pressed" << endl;
		else
			cout << "Right mouse released" << endl;
	}
								break;

	case GLFW_MOUSE_BUTTON_MIDDLE: {
		if (action == GLFW_PRESS)
			cout << "Middle mouse pressed" << endl;
		else
			cout << "Middle mouse released" << endl;
	}
								 break;

	default: {
		cout << "Mouse event wasn't handled" << endl;
		break;
	}
	}
}

//Flips image vertically. Needed because opengl loads it differently
void ImageFlipVert(unsigned char* image, int width, int height, int channels) {
	for (int i = 0; i < height / 2; ++i) {
		int index1 = i * width * channels;
		int index2 = (height - 1 - i) * width * channels;
		for (int k = width * channels; k >> 0; --k) {
			unsigned char temp = image[index1];
			image[index1] = image[index2];
			image[index2] = temp;
			++index1;
			++index2;
		}
	}
}




/*Keeping large blocks of the same func call in one location*/
void MainMeshCreation() {
	MeshCreation(textureCube);

	MeshCreation(tableTop);
	MeshCreation(frontRightLeg);
	MeshCreation(frontLeftLeg);
	MeshCreation(backRightLeg);
	MeshCreation(backLeftLeg);

	MeshCreation(plane);

	MeshCreation(couchFRLeg);
	MeshCreation(couchBRLeg);
	MeshCreation(couchFLLeg);
	MeshCreation(couchBLLeg);
	MeshCreation(couchLeftArm);
	MeshCreation(couchBackSupport);
	MeshCreation(couchRightArm);
	MeshCreation(couchSeatSupport);
	MeshCreation(couchLeftCushion);
	MeshCreation(couchRightCushion);

	MeshCreation(laptopBase);
	MeshCreation(laptopBack);
	MeshCreation(laptopScreen);

	MeshCreation(longCouchFRLeg);
	MeshCreation(longCouchBRLeg);
	MeshCreation(longCouchFLLeg);
	MeshCreation(longCouchBLLeg);
	MeshCreation(longCouchLeftArm);
	MeshCreation(longCouchBack);
	MeshCreation(longCouchRightArm);
	MeshCreation(longCouchBase);
	MeshCreation(longCouchCushion1);
	MeshCreation(longCouchCushion2);
	MeshCreation(longCouchCushion3);
	MeshCreation(longCouchCushion4);

	MeshCreation(smallTableBLLeg);
	MeshCreation(smallTableBRLeg);
	MeshCreation(smallTableFLLeg);
	MeshCreation(smallTableFRLeg);
	MeshCreation(smallTableTop);
	MeshCreation(smallTableMid);

	MeshCreation(cabinetBack);
	MeshCreation(cabinetBase);
	MeshCreation(cabinetLeftSide);
	MeshCreation(cabinetRightSide);
	MeshCreation(cabinetTop);
	MeshCreation(cabinetShelf1);
	MeshCreation(cabinetShelf2);
	MeshCreation(cabinetShelf3);
	MeshCreation(cabinetInnerWall1);
	MeshCreation(cabinetInnerWall2);

	MeshCreation(decor1);
	MeshCreation(decor2);
	MeshCreation(decor3);

	MeshCreation(testCyl);

	MeshCreation(lightCube1);
	MeshCreation(lightCube2);
}

void MainMeshDeletion() {
	MeshDeletion(textureCube);

	MeshDeletion(tableTop);
	MeshDeletion(frontRightLeg);
	MeshDeletion(frontLeftLeg);
	MeshDeletion(backRightLeg);
	MeshDeletion(backLeftLeg);

	MeshDeletion(plane);

	MeshDeletion(couchFRLeg);
	MeshDeletion(couchBRLeg);
	MeshDeletion(couchFLLeg);
	MeshDeletion(couchBLLeg);
	MeshDeletion(couchSeatSupport);
	MeshDeletion(couchBackSupport);
	MeshDeletion(couchRightArm);
	MeshDeletion(couchSeatSupport);
	MeshDeletion(couchLeftCushion);
	MeshDeletion(couchRightCushion);

	MeshDeletion(laptopBack);
	MeshDeletion(laptopScreen);
	MeshDeletion(laptopBase);

	MeshDeletion(longCouchFRLeg);
	MeshDeletion(longCouchBRLeg);
	MeshDeletion(longCouchFLLeg);
	MeshDeletion(longCouchBLLeg);
	MeshDeletion(longCouchLeftArm);
	MeshDeletion(longCouchBack);
	MeshDeletion(longCouchRightArm);
	MeshDeletion(longCouchBase);
	MeshDeletion(longCouchCushion1);
	MeshDeletion(longCouchCushion2);
	MeshDeletion(longCouchCushion3);
	MeshDeletion(longCouchCushion4);

	MeshDeletion(smallTableBLLeg);
	MeshDeletion(smallTableBRLeg);
	MeshDeletion(smallTableFLLeg);
	MeshDeletion(smallTableFRLeg);
	MeshDeletion(smallTableTop);
	MeshDeletion(smallTableMid);

	MeshDeletion(cabinetBack);
	MeshDeletion(cabinetBase);
	MeshDeletion(cabinetLeftSide);
	MeshDeletion(cabinetRightSide);
	MeshDeletion(cabinetTop);
	MeshDeletion(cabinetShelf1);
	MeshDeletion(cabinetShelf2);
	MeshDeletion(cabinetShelf3);
	MeshDeletion(cabinetInnerWall1);
	MeshDeletion(cabinetInnerWall2);

	MeshDeletion(decor1);
	MeshDeletion(decor2);
	MeshDeletion(decor3);

	MeshDeletion(testCyl);

	MeshDeletion(lightCube1);
	MeshDeletion(lightCube2);
}

void MainObjectScaleTransRotate() {
	//Object to help with texture allignment
	ObjectScaleTransRotate(5.0f, 5.0f, 5.0f, 0.0f, 5.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, textureCube);

	//Objects for the table
	ObjectScaleTransRotate(4.0f, 0.2f, 4.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, tableTop);
	ObjectScaleTransRotate(0.2f, 1.0f, 0.2f, -1.5f, -1.5f, -3.45f, 1.0f, 1.0f, 1.0f, 0.0f, frontRightLeg);
	ObjectScaleTransRotate(0.2f, 1.0f, 0.2f, 1.5f, -1.5f, -3.45f, 1.0f, 1.0f, 1.0f, 0.0f, frontLeftLeg);
	ObjectScaleTransRotate(0.2f, 1.0f, 0.2f, 1.5f, -1.5f, -0.35f, 1.0f, 1.0f, 1.0f, 0.0f, backLeftLeg);
	ObjectScaleTransRotate(0.2f, 1.0f, 0.2f, -1.5f, -1.5f, -0.35f, 1.0f, 1.0f, 1.0f, 0.0f, backRightLeg);

	//Object for the plane
	ObjectScaleTransRotate(15.0f, 0.05f, 15.0f, 0.0f, -2.0f, 3.85f, 1.0f, 1.0f, 1.0f, 0.0f, plane);
	ObjectScaleTransRotate(15.0f, 0.05f, 15.0f, 0.0f, -2.0f, 3.85f, 1.0f, 1.0f, 1.0f, 0.0f, lightCube1);

	//Couch
	ObjectScaleTransRotate(5.0f, 0.4f, 1.5f, 0.0f, -1.3f, 3.5f, 1.0f, 1.0f, 1.0f, 0.0f, couchSeatSupport);
	ObjectScaleTransRotate(4.99f, 0.2f, 1.2f, 0.0f, -1.25f, 3.5f, 1.0f, 0.0f, 0.0f, 1.7f, couchBackSupport);
	ObjectScaleTransRotate(0.1f, 0.5f, 0.1f, -2.2f, -1.75f, 3.3f, 1.0f, 1.0f, 1.0f, 0.0f, couchBRLeg);
	ObjectScaleTransRotate(0.1f, 0.5f, 0.1f, -2.2f, -1.75f, 2.3f, 1.0f, 1.0f, 1.0f, 0.0f, couchBLLeg);
	ObjectScaleTransRotate(0.1f, 0.5f, 0.1f, 2.2f, -1.75f, 3.3f, 1.0f, 1.0f, 1.0f, 0.0f, couchFLLeg);
	ObjectScaleTransRotate(0.1f, 0.5f, 0.1f, 2.2f, -1.75f, 2.3f, 1.0f, 1.0f, 1.0f, 0.0f, couchFRLeg);
	ObjectScaleTransRotate(0.2f, 0.9f, 1.6f, 2.5f, -1.0f, 3.5f, 1.0f, 1.0f, 1.0f, 0.0f, couchRightArm);
	ObjectScaleTransRotate(0.2f, 0.9f, 1.6f, -2.5f, -1.0f, 3.5f, 1.0f, 1.0f, 1.0f, 0.0f, couchLeftArm);
	ObjectScaleTransRotate(2.3f, 0.3f, 1.4f, -1.2f, -1.1f, 3.5f, 1.0f, 1.0f, 1.0f, 0.0f, couchRightCushion);
	ObjectScaleTransRotate(2.3f, 0.3f, 1.4f, 1.2f, -1.1f, 3.5f, 1.0f, 1.0f, 1.0f, 0.0f, couchLeftCushion);

	//Laptop
	ObjectScaleTransRotate(0.65f, 0.05f, 0.4f, -0.10f, -0.87f, -0.10f, 1.0f, 1.0f, 1.0f, 0.0f, laptopBase);
	ObjectScaleTransRotate(0.65f, 0.05f, 0.4f, -0.10f, -0.87f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, laptopBack);
	ObjectScaleTransRotate(0.6f, 0.05f, 0.35f, -0.10f, -0.85f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, laptopScreen);
	ObjectScaleTransRotate(0.6f, 0.05f, 0.35f, -0.10f, -0.85f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, lightCube2);

	//Long Couch
	ObjectScaleTransRotate(1.5f, 0.4f, 7.0f, 5.0f, -1.3f, 0.25f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchBase);
	ObjectScaleTransRotate(0.15f, 0.5f, 0.15f, 5.5f, -1.75f, -6.25f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchBLLeg);
	ObjectScaleTransRotate(0.15f, 0.5f, 0.15f, 4.5f, -1.75f, -6.25f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchFLLeg);
	ObjectScaleTransRotate(0.15f, 0.5f, 0.15f, 5.5f, -1.75f, -0.25f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchFRLeg);
	ObjectScaleTransRotate(0.15f, 0.5f, 0.15f, 4.5f, -1.75f, -0.25f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchBRLeg);
	ObjectScaleTransRotate(1.48f, 0.4f, 7.0f, 6.0f, -0.5f, 0.25f, 0.0f, 0.0f, 1.0f, 1.4f, longCouchBack);
	ObjectScaleTransRotate(1.4f, 0.3f, 1.5f, 5.0f, -1.1f, -0.05f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchCushion1);
	ObjectScaleTransRotate(1.4f, 0.3f, 1.5f, 5.0f, -1.1f, -4.85f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchCushion2);
	ObjectScaleTransRotate(1.4f, 0.3f, 1.5f, 5.0f, -1.1f, -3.22f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchCushion3);
	ObjectScaleTransRotate(1.4f, 0.3f, 1.5f, 5.0f, -1.1f, -1.65f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchCushion4);
	ObjectScaleTransRotate(1.7f, 1.0f, 0.2f, 5.0f, -1.0f, 0.29f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchLeftArm);
	ObjectScaleTransRotate(1.7f, 1.0f, 0.2f, 5.0f, -1.0f, -6.61f, 1.0f, 1.0f, 1.0f, 0.0f, longCouchRightArm);

	//Smaller Table
	ObjectScaleTransRotate(1.0f, 0.05f, 1.5f, -3.2f, -1.7f, 3.5f, 1.0f, 1.0f, 1.0f, 0.0f, smallTableMid);
	ObjectScaleTransRotate(1.0f, 0.05f, 1.5f, -3.2f, -1.3f, 3.5f, 1.0f, 1.0f, 1.0f, 0.0f, smallTableTop);
	ObjectScaleTransRotate(0.05f, 0.7f, 0.05f, -2.8f, -1.66f, 3.45f, 1.0f, 1.0f, 1.0f, 0.0f, smallTableBRLeg);
	ObjectScaleTransRotate(0.05f, 0.7f, 0.05f, -3.6f, -1.66f, 3.45f, 1.0f, 1.0f, 1.0f, 0.0f, smallTableBLLeg);
	ObjectScaleTransRotate(0.05f, 0.7f, 0.05f, -3.6f, -1.66f, 2.15f, 1.0f, 1.0f, 1.0f, 0.0f, smallTableFRLeg);
	ObjectScaleTransRotate(0.05f, 0.7f, 0.05f, -2.8f, -1.66f, 2.15f, 1.0f, 1.0f, 1.0f, 0.0f, smallTableFLLeg);

	//Cabinet
	ObjectScaleTransRotate(9.0f, 0.2f, 1.0f, -3.0f, -1.9f, -10.0f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetBase);
	ObjectScaleTransRotate(9.0f, 0.6f, 1.0f, -3.0f, 1.9f, -10.0f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetTop);
	ObjectScaleTransRotate(9.0f, 0.05f, 1.0f, -3.0f, 0.8f, -10.0f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetShelf1);
	ObjectScaleTransRotate(9.0f, 0.05f, 1.0f, -3.0f, 0.1f, -10.0f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetShelf2);
	ObjectScaleTransRotate(9.0f, 0.05f, 1.0f, -3.0f, -0.7f, -10.0f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetShelf3);
	ObjectScaleTransRotate(9.0f, 4.0f, 0.05f, -3.0f, 0.0f, -11.0f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetBack);
	ObjectScaleTransRotate(0.2f, 4.0f, 0.8f, -7.4f, 0.0f, -10.1f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetLeftSide);
	ObjectScaleTransRotate(0.2f, 4.0f, 0.8f, 1.4f, 0.0f, -10.1f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetRightSide);
	ObjectScaleTransRotate(0.2f, 4.0f, 0.8f, -4.5f, 0.0f, -10.1f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetInnerWall1);
	ObjectScaleTransRotate(0.2f, 4.0f, 0.8f, -1.5f, 0.0f, -10.1f, 3.0f, 1.0f, 1.0f, 0.0f, cabinetInnerWall2);

	//Decor pieces
	ObjectScaleTransRotate(0.2f, 0.2f, 0.2f, -1.0f, 0.1f, -10.5f, 3.0f, 1.0f, 1.0f, 0.0f, decor1);
	ObjectScaleTransRotate(0.2f, 0.2f, 0.2f, -4.0f, 0.1f, -10.5f, 3.0f, 1.0f, 1.0f, 0.0f, decor2);
	ObjectScaleTransRotate(0.2f, 0.2f, 0.2f, 0.0f, -0.9f, -1.0f, 3.0f, 1.0f, 1.0f, 0.0f, decor3);

	ObjectScaleTransRotate(1.0f, 1.0f, 1.0f, 0.0f, 5.0f, -5.0f, 3.0f, 1.0f, 1.0f, 0.0f, testCyl);
}

void MainDrawObject(GLint viewPosLoc, glm::mat4 view, glm::mat4 projection) {
	//DrawObject(textureCube, textures.screen, viewPosLoc, view, projection);

	DrawObject(plane, textures.navyCarpet, viewPosLoc, view, projection);

	DrawObject(tableTop, textures.greyPlastic, viewPosLoc, view, projection);
	DrawObject(frontRightLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(frontLeftLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(backRightLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(backLeftLeg, textures.woodenLegs, viewPosLoc, view, projection);

	DrawObject(couchBLLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(couchBRLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(couchFLLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(couchFRLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(couchSeatSupport, textures.blackFabric, viewPosLoc, view, projection);
	DrawObject(couchLeftCushion, textures.blueFabric, viewPosLoc, view, projection);
	DrawObject(couchRightCushion, textures.blueFabric, viewPosLoc, view, projection);
	DrawObject(couchRightArm, textures.blackFabric, viewPosLoc, view, projection);
	DrawObject(couchLeftArm, textures.blackFabric, viewPosLoc, view, projection);
	DrawObject(couchBackSupport, textures.blackFabric, viewPosLoc, view, projection);

	DrawObject(laptopBase, textures.blackPlastic, viewPosLoc, view, projection);
	DrawObject(laptopBack, textures.blackPlastic, viewPosLoc, view, projection);
	DrawObject(laptopScreen, textures.screen, viewPosLoc, view, projection);

	DrawObject(longCouchBase, textures.blackFabric, viewPosLoc, view, projection);
	DrawObject(longCouchBLLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(longCouchBRLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(longCouchFRLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(longCouchFLLeg, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(longCouchBack, textures.blackFabric, viewPosLoc, view, projection);
	DrawObject(longCouchCushion1, textures.blueFabric, viewPosLoc, view, projection);
	DrawObject(longCouchCushion2, textures.blueFabric, viewPosLoc, view, projection);
	DrawObject(longCouchCushion3, textures.blueFabric, viewPosLoc, view, projection);
	DrawObject(longCouchCushion4, textures.blueFabric, viewPosLoc, view, projection);
	DrawObject(longCouchLeftArm, textures.blackFabric, viewPosLoc, view, projection);
	DrawObject(longCouchRightArm, textures.blackFabric, viewPosLoc, view, projection);

	DrawObject(smallTableTop, textures.glass, viewPosLoc, view, projection);
	DrawObject(smallTableMid, textures.glass, viewPosLoc, view, projection);
	DrawObject(smallTableBRLeg, textures.greyMetal, viewPosLoc, view, projection);
	DrawObject(smallTableBLLeg, textures.greyMetal, viewPosLoc, view, projection);
	DrawObject(smallTableFRLeg, textures.greyMetal, viewPosLoc, view, projection);
	DrawObject(smallTableFLLeg, textures.greyMetal, viewPosLoc, view, projection);

	DrawObject(cabinetBase, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetTop, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetShelf1, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetShelf2, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetShelf3, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetBack, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetLeftSide, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetRightSide, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetInnerWall1, textures.woodenLegs, viewPosLoc, view, projection);
	DrawObject(cabinetInnerWall2, textures.woodenLegs, viewPosLoc, view, projection);

	DrawObject(decor1, textures.blueFabric, viewPosLoc, view, projection);
	DrawObject(decor2, textures.brick, viewPosLoc, view, projection);
	DrawObject(decor3, textures.brick, viewPosLoc, view, projection);

	DrawObject(testCyl, textures.brick, viewPosLoc, view, projection);
}

void MainTextureCreation() {
	TextureCreation("../../resources/textures/brick.png", textures.brick);
	TextureCreation("../../resources/textures/blackfabric.png", textures.blackFabric);
	TextureCreation("../../resources/textures/navycarpet.png", textures.navyCarpet);
	TextureCreation("../../resources/textures/greyplastic.png", textures.greyPlastic);
	TextureCreation("../../resources/textures/wood.png", textures.woodenLegs);
	TextureCreation("../../resources/textures/bluefabric.png", textures.blueFabric);
	TextureCreation("../../resources/textures/screen.png", textures.screen);
	TextureCreation("../../resources/textures/glass.png", textures.glass);
	TextureCreation("../../resources/textures/greymetal.png", textures.greyMetal);
	TextureCreation("../../resources/textures/blackplastic.png", textures.blackPlastic);
}

void MainTextureDeletion() {
	TextureDeletion(textures.brick);
	TextureDeletion(textures.blackFabric);
	TextureDeletion(textures.blueFabric);
	TextureDeletion(textures.greyPlastic);
	TextureDeletion(textures.navyCarpet);
	TextureDeletion(textures.woodenLegs);
	TextureDeletion(textures.blackPlastic);
	TextureDeletion(textures.glass);
	TextureDeletion(textures.screen);
	TextureDeletion(textures.greyMetal);
}
