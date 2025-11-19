#include "Scene1.h"
#include "GL\glew.h"

#include "shader.hpp"
#include "Application.h"
#include "MeshBuilder.h"
#include "KeyboardController.h"

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

Scene1::Scene1()
{
}

Scene1::~Scene1()
{
}

void Scene1::Init()
{

	camera.Init(45.f, 45.f, 10.f);

	// Set background color to dark blue
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	//Enable depth buffer and depth testing
	glEnable(GL_DEPTH_TEST);

	//Enable back face culling
	glEnable(GL_CULL_FACE);

	//Default to fill mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Generate a default VAO for now
	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);

	// Load the shader programs
	m_programID = LoadShaders("Shader//TransformVertexShader.vertexshader",
								"Shader//SimpleFragmentShader.fragmentshader");
	glUseProgram(m_programID);

	// Get a handle for our "MVP" uniform
	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");

	// Init VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = nullptr;
	}

	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
	meshList[GEO_QUAD] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f), 1.f);
	meshList[GEO_SPHERE] = MeshBuilder::GenerateSphere("Sphere", glm::vec3(1.f, 1.f, 1.f), 1.f, 12, 12);
	meshList[GEO_TORUS] = MeshBuilder::GenerateTorus("Torus", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 12, 12);
}

void Scene1::Update(double dt)
{
	// Check for key press, you can add more for interaction
	HandleKeyPress();

	camera.Update(dt);
}

void Scene1::Render()
{
	// Clear color buffer every frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup Model View Projection matrix
	// Define the model matrix and set it to identity
	glm::mat4 model = glm::mat4(1.0f);
	// Define the view matrix and set it with camera position, target position and up direction
	glm::mat4 view = glm::lookAt(
		glm::vec3(camera.position.x, camera.position.y, camera.position.z),
		glm::vec3(camera.target.x, camera.target.y, camera.target.z),
		glm::vec3(0.f, 1.f, 0.f)
	);
	//Define the projection matrix either in perspective ...
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	// or orthographic ...
	//glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 1000.f);
	
	//glm::mat4 projection = glm::mat4(1.f);
	switch (projType) {
	case 0: // ORTHOGRAPHICS
		// Add codes here to setup orthographic matrix
		glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 1000.f);
		break;
	default: // PERSPECTIVE
		// Add codes here to setup perspective matrix
		glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	}

	// Calculate the Model-View-Project matrix
	glm::mat4 MVP = projection * view * model;
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE,
		glm::value_ptr(MVP));

	// Render objects
	meshList[GEO_AXES]->Render();
	//meshList[GEO_QUAD]->Render();
	//meshList[GEO_SPHERE]->Render();
	meshList[GEO_TORUS]->Render();
}

void Scene1::Exit()
{
	// Cleanup VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if (meshList[i])
		{
			delete meshList[i];
		}
	}
	glDeleteVertexArrays(1, &m_vertexArrayID);
	glDeleteProgram(m_programID);
}

void Scene1::HandleKeyPress() 
{
	if (Application::IsKeyPressed(0x31))
	{
		// Key press to enable culling
		glEnable(GL_CULL_FACE);
	}
	if (Application::IsKeyPressed(0x32))
	{
		// Key press to disable culling
		glDisable(GL_CULL_FACE);
	}
	if (Application::IsKeyPressed(0x33))
	{
		// Key press to enable fill mode for the polygon
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //default fill mode
	}
	if (Application::IsKeyPressed(0x34))
	{
		// Key press to enable wireframe mode for the polygon
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe mode
	}
	if (Application::IsKeyPressed('P')) {
		if (projType == 0)
			projType = 1; // Perspective
		else
			projType = 0; // Orthographic
	}
}
