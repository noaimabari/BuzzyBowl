/*

Author: Noaima Bari
Class: ECE6122
Last Date Modified: 6 Dec 2021
Description:
ECE 6122 Final Project Implementation using OpenGL and Multithreading

References: https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/

*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include <vector>
#include <string>

using namespace std;



enum class ImageType :int8_t { eBITMAP, eDDS };

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include "ECE_UAV.h"



bool drawObject(GLuint& Texture,
	GLuint& TextureID,
	GLuint& Vertexbuffer,
	GLuint& UVbuffer,
	GLuint& Normalbuffer,
	GLuint& Elementbuffer,
	uint& nCount);

bool setupObject(ImageType imageType,
	const std::string& ImageFilename,
	const std::string& ObjectFilename,
	GLuint& Texture,
	GLuint& Vertexbuffer,
	GLuint& UVbuffer,
	GLuint& Normalbuffer,
	GLuint& Elementbuffer,
	uint& nCount);


ECE_UAV ece_uav[15];
int main(void)
{
	

	double uav_pos[3];
	// Initialize the x, y, z locations
	double x[15] = { -135.0f, -68.75f, -2.5f, 63.75f, 130.0f, -135.0f, -68.75f, -2.5f, 63.75f, 130.0f, -135.0f, -68.75f, -2.5f, 63.75f, 130.0f };
	double y[15] = { 60.0f, 60.0f, 60.0f, 60.0f, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -60.0f, -60.0f, -60.0f, -60.0f, -60.0f };
	double z[15] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	for (int i = 0; i < 15; i++)
	{
		uav_pos[0] = x[i];
		uav_pos[1] = y[i];
		uav_pos[2] = z[i];

		ece_uav[i].position(uav_pos);

	}

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		//getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		//getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		//getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	//glfwPollEvents();
	//glfwSetCursorPos(window, 1024/2, 768/2);

	GLuint footballTexture;
	GLuint footballVertexbuffer;
	GLuint footballUVbuffer;
	GLuint footballNormalbuffer;
	GLuint footballElementbuffer;
	uint nCountFootballField = 0;

	setupObject(ImageType::eBITMAP,
		"ff.bmp",
		"footballfield.obj",
		footballTexture,
		footballVertexbuffer,
		footballUVbuffer,
		footballNormalbuffer,
		footballElementbuffer,
		nCountFootballField);

	GLuint sphereTexture;
	GLuint sphereVertexBuffer;
	GLuint sphereUVBuffer;
	GLuint sphereNormalBuffer;
	GLuint sphereElementBuffer;
	uint nCountSphere = 0;

	setupObject(ImageType::eBITMAP,
		"uvmap.DDS",
		"sphere.obj",
		sphereTexture,
		sphereVertexBuffer,
		sphereUVBuffer,
		sphereNormalBuffer,
		sphereElementBuffer,
		nCountSphere);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Enable blending
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	GLuint programID2 = LoadShaders("StandardShading.vertexshader", "StandardShadingColor.fragmentshader"); //



	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Added extra now 
	GLuint MatrixID2 = glGetUniformLocation(programID2, "MVP");
	GLuint ViewMatrixID2 = glGetUniformLocation(programID2, "V");
	GLuint ModelMatrixID2 = glGetUniformLocation(programID2, "M");



	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");
	GLuint Texture2 = loadDDS("uvmap.DDS");


	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint TextureID2 = glGetUniformLocation(programID2, "myTextureSampler");


	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	
	// added extra now

	glUseProgram(programID2);
	GLuint LightID2 = glGetUniformLocation(programID2, "LightPosition_worldspace");

	// Generate a handle for our color variant

	GLint color_variant = glGetUniformLocation(programID2, "colorVariant");


	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	float rotAngle = (90.0f / (2.0f * 3.14156f)); // extra

	// start your threads

	for (int i = 0; i < 15; i++)
	{
		ece_uav[i].start();
		cout << "Started thread " << i << endl;
	}

	double a, b, c;
	double D, touchTime, elapsedTime;
	bool firstTime = true, getOut = false;
	a = 1;
	b = 1;
	c = 1;
	bool flag = false;

	do {

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}
		if (flag == true)
		{
			a = a / 2;
			b = b / 2;
			c = c / 2;
			flag = false;
		}
		else
		{
			a = a * 2;
			b = b * 2;
			c = c * 2;
			flag = true;
		}
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();


		////// Start of the rendering of the first object //////

		// Use our shader
		glUseProgram(programID);

		glm::vec3 lightPos = glm::vec3(0, -250, 200); // extra: changes
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);

		// Draw the football field (extra)
		drawObject(footballTexture,
			TextureID,
			footballVertexbuffer,
			footballUVbuffer,
			footballNormalbuffer,
			footballElementbuffer,
			nCountFootballField);


		// Draw the sphere
		glm::vec3 lightPos2 = glm::vec3(0, -250, 200); // extra: changes
		glUniform3f(LightID, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		glm::mat4 ModelMatrix0 = glm::mat4(1.0);

		ModelMatrix0 = glm::translate(ModelMatrix0, glm::vec3(0.0f, 0.0f, 135.0f));
		ModelMatrix0 = glm::rotate(ModelMatrix0, rotAngle, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix0 = glm::scale(ModelMatrix0, glm::vec3(5.3f, 5.3f, 5.3f));
		glm::mat4 MVP0 = ProjectionMatrix * ViewMatrix * ModelMatrix0;

		// Extra : Send our transformation to the currently bound shader
		// in the MVP uniform

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP0[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix0[0][0]);


		drawObject(sphereTexture,
			TextureID,
			sphereVertexBuffer,
			sphereUVBuffer,
			sphereNormalBuffer,
			sphereElementBuffer,
			nCountSphere);


		glUseProgram(programID);
		// Extra 
		ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(50.0f, 0.0f, 80.0f));
		ModelMatrix1 = glm::rotate(ModelMatrix1, rotAngle, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix1 = glm::scale(ModelMatrix1, glm::vec3(5.0f, 5.0f, 5.0f));
		MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Extra : Send our transformation to the currently bound shader
		// in the MVP uniform

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);




		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		/*glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);*/




		

		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		// need to do something similar for our final project also loop through and move mode
		ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(100.0f, 20.0f, 60.0f));
		ModelMatrix2 = glm::rotate(ModelMatrix2, rotAngle, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix2 = glm::scale(ModelMatrix2, glm::vec3(5.0f, 5.0f, 5.0f));
		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);


		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);



		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		////// End of rendering of the second object //////


		// Rendering the multiple UAVs
		
		glUseProgram(programID2);

		glm::vec3 cc = glm::vec3(a, b, c); // extra: changes
		glUniform3f(color_variant, cc.x, cc.y, cc.z);

		glm::vec3 lightPos3 = glm::vec3(0, -250, 200); // extra: changes

		glUniform3f(LightID2, lightPos3.x, lightPos3.y, lightPos3.z);
		glUniformMatrix4fv(ViewMatrixID2, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		for (int i = 0; i < 15; i++)
		{
			ece_uav[i].getPosition(uav_pos);
			
			D = uav_pos[0] * uav_pos[0] + uav_pos[1] * uav_pos[1] + (uav_pos[2] - 135) * (uav_pos[2] - 135);

			if (D <= 400 && firstTime == true)
			{
				touchTime = glfwGetTime();
				cout << "Touched it at time : " << touchTime << endl;
				firstTime = false;
			}
			if (D <= 400 && firstTime == false)
			{
				elapsedTime = glfwGetTime() - touchTime;
				if (elapsedTime >= 60)
				{
					cout << "60 seconds have completed. Stopping simulation .. " << endl;
					getOut = true;
					break;
				}
			}
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture2);
			//// Set our "myTextureSampler" sampler to use Texture Unit 0
			glUniform1i(TextureID2, 0);

			glm::mat4 ModelMatrixUAV = glm::mat4(1.0);
			// need to do something similar for our final project also loop through and move mode
			ModelMatrixUAV = glm::translate(ModelMatrixUAV, glm::vec3(uav_pos[0], uav_pos[1], uav_pos[2]));
			ModelMatrixUAV = glm::rotate(ModelMatrixUAV, rotAngle, glm::vec3(1.0f, 0.0f, 0.0f));
			ModelMatrixUAV = glm::scale(ModelMatrixUAV, glm::vec3(3.0f, 3.0f, 3.0f));
			glm::mat4 MVPN = ProjectionMatrix * ViewMatrix * ModelMatrixUAV;

	
			
			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVPN[0][0]);
			glUniformMatrix4fv(ModelMatrixID2, 1, GL_FALSE, &ModelMatrixUAV[0][0]);


			// The rest is exactly the same as the first object

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);


		}

		if (getOut == true) break;


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Stop the threads

	for (int i = 0; i < 15; i++)
	{
		ece_uav[i].stop();
	}

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteProgram(programID2);
	glDeleteTextures(1, &Texture);
	glDeleteTextures(1, &Texture2);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

bool setupObject(ImageType imageType,
	const std::string& ImageFilename,
	const std::string& ObjectFilename,
	GLuint& Texture,
	GLuint& Vertexbuffer,
	GLuint& UVbuffer,
	GLuint& Normalbuffer,
	GLuint& Elementbuffer,
	uint& nCount)
{

	// Function to set up an object for drawing in OpenGL
	
	// Load the texture using any two methods
	if (imageType == ImageType::eBITMAP)
	{
		Texture = loadBMP_custom(ImageFilename.c_str());
	}
	else
	{
		Texture = loadDDS(ImageFilename.c_str());
	}

	// Read our obj files
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(ObjectFilename.c_str(), vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	nCount = indices.size();

	// Load into a VBO
	glGenBuffers(1, &Vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, Vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &UVbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &Normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, Normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &Elementbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, Elementbuffer);
	glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	return true;
}

bool drawObject(GLuint& Texture,
	GLuint& TextureID,
	GLuint& Vertexbuffer,
	GLuint& UVbuffer,
	GLuint& Normalbuffer,
	GLuint& Elementbuffer,
	uint& nCount)
{
	// Function to draw an object using OpennGL

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, Vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, Normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Elementbuffer);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		nCount,    // count
		GL_UNSIGNED_SHORT,   // type
		(void*)0           // element array buffer offset
	);


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	return true;

}