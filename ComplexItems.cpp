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
#include "Cylinder.h"

using namespace std;
float counter = 0.0f;

#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

//Unnamed namespace
namespace {
	const char* const Window_Title = "Texturing";
	const int Window_Height = 600;
	const int Window_Width = 800;

	GLFWwindow* mainWindow = nullptr;
	GLuint mainProgramID;
	GLuint lightProgramID;	

	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 lightScale(0.3f);
	glm::vec3 objectColor(1.0f, 0.2f, 0.0f);
	glm::vec3 lightPos(1.5f, 0.5f, 3.0f);

	bool isLampMoving = false;

	//Struct to hold object variables
	struct Cube {
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
			-0.5f,  -0.5f,  0.0f,  0.0f, 0.0f,  1.0f,   1.0f, 0.0f, // 2
			-0.5f,   0.5f,  0.0f,  0.0f, 0.0f,  1.0f,	0.0f, 0.0f, // 3

			 0.5f,  -0.5f, -1.0f,  -1.0f, 0.0f,  0.0f,	1.0f, 1.0f, // 4
			 0.5f,   0.5f, -1.0f,  -1.0f, 0.0f,  0.0f,	0.0f, 1.0f, // 5
			-0.5f,   0.5f, -1.0f,  1.0f, 0.0f,  0.0f,	1.0f, 0.0f, // 6
			-0.5f,  -0.5f, -1.0f,  1.0f, 0.0f,  0.0f,	0.0f, 1.0f  // 7
		};
	};

	//All declarations for objects in scene
	Cube tableTop, frontRightLeg, frontLeftLeg, backRightLeg, backLeftLeg, plane;
	Cube chairFRLeg, chairFLLeg, chairBRLeg, chairBLLeg, chairBackLeft, chairBackRight, chairBackSupport, chairSeat;
	Cube lightObject;

	struct Texture {
		GLuint greyPlastic;
		GLuint greyCarpet;
		GLuint blackFabric;
		GLuint woodenLegs;
	};

	Texture textures;
	GLUquadric *quadObj = gluNewQuadric();
	GLint attribVertexPosition;
	GLint attribVertexNormal;
	GLint attribVertexTexCoord;
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
void MeshCreation(Cube& objMesh);
void MeshDeletion(Cube& objMesh);
void RenderFrame();
bool CreateShaders(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programID);
void DestroyShaders(GLuint programID);
void DrawObject(GLint modelLoc, GLint viewLoc, GLint projLoc, GLint objectColorLoc, GLint lightColorLoc, GLint lightPosLoc, GLint viewPosLoc, Cube obj, GLuint texture, glm::mat4 view, glm::mat4 projection);
void MousePositon(GLFWwindow* window, double xPos, double zPos);
void MouseScroll(GLFWwindow* window, double xSet, double zSet);
void MouseButton(GLFWwindow* window, int button, int action, int mods);
void ObjectScaleTransRotate(Cube& obj, float sX, float sY, float sZ, float tX, float tY, float tZ, float rX, float rY, float rZ, float rMulti);
void MainMeshCreation();
void MainMeshDeletion();
void MainTextureCreation();
void MainTextureDeletion();
void TextureCreation(const char* fileName, GLuint& texture);
void TextureDeletion(GLuint texture);
void ImageFlipVert(unsigned char* image, int width, int height, int channels);
void Init();

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
	vertexTextureCoordinate = textureCoordinate;
	vertexNorm = mat3(transpose(inverse(model))) * normal;
}
);

//Fragment shader
const GLchar* objectFragmentShader = GLSL(440,
	in vec3 vertexNorm;
	in vec3 vertexFragPos;
	in vec2 vertexTextureCoordinate;

	out vec4 fragmentColor;

	uniform vec3 objectColor;
	uniform vec3 lightColor;
	uniform vec3 lightPos;
	uniform vec3 viewPosition;
	uniform sampler2D textureBase;

void main()
{
	//Calculate Ambient
	float ambientStrength = 0.3f;
	vec3 ambient = ambientStrength * lightColor;

	//Calculate Diffuse
	vec3 norm = normalize(vertexNorm);
	vec3 lightDirection = normalize(lightPos - vertexFragPos);
	float impact = max(dot(norm, lightDirection), 0.0);
	vec3 diffuse = impact * lightColor;

	//Calculate Specular
	float specularIntensity = 0.8f;
	float highlightSize = 16.0f;
	vec3 viewDir = normalize(viewPosition - vertexFragPos);
	vec3 reflectDir = reflect(-lightDirection, norm);


	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	vec3 specular = specularIntensity * specularComponent * lightColor;

	vec4 textureColor = texture(textureBase, vertexTextureCoordinate);
	vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

	fragmentColor = vec4(phong, 1.0);
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
		//return EXIT_FAILURE;
	}
	if (!CreateShaders(lightVertexShader, lightFragmentShader, lightProgramID)) {
		cout << "Lighting shader failure" << endl;
		//return EXIT_FAILURE;
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
void MeshCreation(Cube& obj) {
	const GLuint fPerVer = 3;
	const GLuint fPerNorm = 3;
	const GLuint fPerText = 2;

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

//Code needed for creating a frame
void RenderFrame() {

	//Endables depth rendering
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Objects for the table
	ObjectScaleTransRotate(tableTop, 2.0f, 0.2f, 2.0f, 0.0f, 0.0f, 0.0f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(frontRightLeg, 0.2f, 2.0f, 0.2f, -0.75f, -1.0f, -0.1f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(frontLeftLeg, 0.2f, 2.0f, 0.2f, -0.75f, -1.0f, -1.7f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(backRightLeg, 0.2f, 2.0f, 0.2f, 0.75f, -1.0f, -0.1f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(backLeftLeg, 0.2f, 2.0f, 0.2f, 0.75f, -1.0f, -1.7f, 1.0, 1.0f, 1.0f, 0.0f);

	//Object for the plane
	ObjectScaleTransRotate(plane, 10.0f, 0.1f, 10.0f, 0.0f, -2.0f, 3.85f, 1.0f, 1.0f, 1.0f, 0.0f);

	//This was added to give a sense of direction in the render
	ObjectScaleTransRotate(chairFRLeg, 0.1f, 1.0f, 0.1f, 1.75f, -1.5f, -0.45f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(chairFLLeg, 0.1f, 1.0f, 0.1f, 1.75f, -1.5f, -1.3f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(chairBRLeg, 0.1f, 1.0f, 0.1f, 2.5f, -1.5f, -0.45f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(chairBLLeg, 0.1f, 1.0f, 0.1f, 2.5f, -1.5f, -1.3f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(chairSeat, 1.2f, 0.1f, 1.1f, 2.1f, -1.0f, -0.375f, 1.0, 1.0f, 1.0f, 0.0f);
	ObjectScaleTransRotate(chairBackRight, 0.1f, 1.1f, 0.1f, 2.5f, -0.4f, -0.45f, 0.0, 0.0f, 1.0f, -0.1f);
	ObjectScaleTransRotate(chairBackLeft, 0.1f, 1.1f, 0.1f, 2.5f, -0.4f, -1.3f, 0.0, 0.0f, 1.0f, -0.1f);
	ObjectScaleTransRotate(chairBackSupport, 0.05f, 0.05f, 0.8f, 2.535f, -0.2f, -0.5f, 0.0, 0.0f, 1.0f, -0.1f);

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

	//Uses user created function to draw the objects 
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, tableTop, textures.greyPlastic, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, frontRightLeg, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, frontLeftLeg, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, backRightLeg, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, backLeftLeg, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, plane, textures.greyCarpet, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, chairBLLeg, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, chairBRLeg, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, chairFLLeg, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, chairFRLeg, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, chairSeat, textures.blackFabric, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, chairBackLeft, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, chairBackRight, textures.woodenLegs, view, projection);
	DrawObject(modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc, chairBackSupport, textures.woodenLegs, view, projection);
	cout << "Calling cylinder Draw -----" << endl;
	Init();
	cout << "Called cylinder Draw -----" << endl;

	glUseProgram(lightProgramID);
	glm::mat4 model = glm::translate(lightPos) * glm::scale(lightScale);

	modelLoc = glGetUniformLocation(lightProgramID, "model");
	viewLoc = glGetUniformLocation(lightProgramID, "view");
	projLoc = glGetUniformLocation(lightProgramID, "projection");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//glDrawElements(GL_TRIANGLES, lightObject.nIndices, GL_UNSIGNED_SHORT, NULL);


	//Deactivates the VAO and shaders
	glBindVertexArray(0);
	glfwSwapBuffers(mainWindow);
}

//Simple func to reduce lines of code. Changes the scale, translation, and rotation of an object with the float passed into it
void ObjectScaleTransRotate(Cube& obj, float sX, float sY, float sZ, float tX, float tY, float tZ, float rX, float rY, float rZ, float rMulti) {
	obj.scale = glm::scale(glm::vec3(sX, sY, sZ));
	obj.translation = glm::translate(glm::vec3(tX, tY, tZ));
	obj.rotation = glm::rotate(rMulti, glm::vec3(rX, rY, rZ));
}

//User create function to draw the object given
void DrawObject(GLint modelLoc, GLint viewLoc, GLint projLoc, GLint objectColorLoc, GLint lightColorLoc, GLint lightPosLoc, GLint viewPosLoc, Cube obj, GLuint texture, glm::mat4 view, glm::mat4 projection) {
	cout << "Beginning of DrawObject" << endl;
	obj.model = obj.translation * obj.rotation * obj.scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//glUniform3f(objectColorLoc, objectColor.r, objectColor.g, objectColor.b);
	glUniform3f(lightColorLoc, lightColor.r, lightColor.b, lightColor.b);
	glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	const glm::vec3 cameraPos = mainCamera.Position;
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	glBindVertexArray(obj.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawElements(GL_TRIANGLES, obj.nIndices, GL_UNSIGNED_SHORT, NULL);
	cout << "End of DrawObject" << endl;
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
void MeshDeletion(Cube& obj) {
	glDeleteVertexArrays(1, &obj.vao);
	glDeleteBuffers(2, obj.vbo);
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
void Init() {
	cout << "Beginning of Cylinder Draw" << endl;
	attribVertexPosition = glGetAttribLocation(mainProgramID, "vertexPosition");
	attribVertexNormal = glGetAttribLocation(mainProgramID, "vertexNormal");
	attribVertexTexCoord = glGetAttribLocation(mainProgramID, "vertexTexCoord");

	Cylinder cylinder;
	cylinder.set(5.0f, 5.0f, 5.0f, 32, 32, true);

	GLuint vboId;
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId); 
	glBufferData(GL_ARRAY_BUFFER, cylinder.getInterleavedVertexSize(), cylinder.getInterleavedVertices(), GL_STATIC_DRAW); 

	GLuint iboId;
	glGenBuffers(1, &iboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder.getIndexSize(), cylinder.getIndices(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	glEnableVertexAttribArray(attribVertexPosition);
	glEnableVertexAttribArray(attribVertexNormal);
	glEnableVertexAttribArray(attribVertexTexCoord);

	int stride = cylinder.getInterleavedStride();
	glVertexAttribPointer(attribVertexPosition, 3, GL_FLOAT, false, stride, 0);
	glVertexAttribPointer(attribVertexNormal, 3, GL_FLOAT, false, stride, (void*)(3 * sizeof(float)));
	glVertexAttribPointer(attribVertexTexCoord, 2, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures.woodenLegs);
	glDrawElements(GL_TRIANGLES, cylinder.getIndexCount(), GL_UNSIGNED_INT, (void*)0);                      

	glDisableVertexAttribArray(attribVertexPosition);
	glDisableVertexAttribArray(attribVertexNormal);
	glDisableVertexAttribArray(attribVertexTexCoord);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	cout << "End of Cylinder Draw" << endl;
}


void MainMeshCreation() {
	MeshCreation(tableTop);
	MeshCreation(frontRightLeg);
	MeshCreation(frontLeftLeg);
	MeshCreation(backRightLeg);
	MeshCreation(backLeftLeg);
	MeshCreation(plane);
	MeshCreation(chairFRLeg);
	MeshCreation(chairBRLeg);
	MeshCreation(chairFLLeg);
	MeshCreation(chairBLLeg);
	MeshCreation(chairBackLeft);
	MeshCreation(chairBackRight);
	MeshCreation(chairBackSupport);
	MeshCreation(chairSeat);
	MeshCreation(lightObject);
}

void MainMeshDeletion() {
	MeshDeletion(tableTop);
	MeshDeletion(frontRightLeg);
	MeshDeletion(frontLeftLeg);
	MeshDeletion(backRightLeg);
	MeshDeletion(backLeftLeg);
	MeshDeletion(plane);
	MeshDeletion(chairFRLeg);
	MeshDeletion(chairBRLeg);
	MeshDeletion(chairFLLeg);
	MeshDeletion(chairBLLeg);
	MeshDeletion(chairBackLeft);
	MeshDeletion(chairBackRight);
	MeshDeletion(chairBackSupport);
	MeshDeletion(chairSeat);
	MeshDeletion(lightObject);
}

void MainTextureCreation() {
	TextureCreation("../../resources/textures/blackfabric.png", textures.blackFabric);
	TextureCreation("../../resources/textures/navycarpet.png", textures.greyCarpet);
	TextureCreation("../../resources/textures/greyplastic.png", textures.greyPlastic);
	TextureCreation("../../resources/textures/wood.png", textures.woodenLegs);
}

void MainTextureDeletion() {
	TextureDeletion(textures.blackFabric);
	TextureDeletion(textures.greyCarpet);
	TextureDeletion(textures.greyPlastic);
	TextureDeletion(textures.woodenLegs);
}
