// ---------------------------------------------------------------
// jin ming's scene
// ---------------------------------------------------------------

#include "Scene01.h"
#include "Mesh.h"
#include "GL\glew.h"

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_inverse.hpp>

//Include GLFW
#include <GLFW/glfw3.h>

#include "shader.hpp"
#include "Application.h"
#include "MeshBuilder.h"
#include "KeyboardController.h"
#include "LoadTGA.h"
#include "MouseController.h"
#include <iostream>

// repo cloning text test

Scene01::Scene01()
{
}

Scene01::~Scene01()
{
}

void Scene01::Init()
{
	// Set background color to dark blue
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	//Enable depth buffer and depth testing
	glEnable(GL_DEPTH_TEST);

	//Enable back face culling (change back to glEnable to allow)
	glDisable(GL_CULL_FACE);

	//Default to fill mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Generate a default VAO for now
	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);

	// Load the shader programs
	//m_programID = LoadShaders("Shader//Texture.vertexshader", "Shader//Texture.fragmentshader");
	m_programID = LoadShaders("Shader//Texture.vertexshader", "Shader//Text.fragmentshader");
	glUseProgram(m_programID);

	// Get a handle for our "MVP" uniform
	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");
	m_parameters[U_MODELVIEW] = glGetUniformLocation(m_programID, "MV");
	m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE] = glGetUniformLocation(m_programID, "MV_inverse_transpose");
	m_parameters[U_MATERIAL_AMBIENT] = glGetUniformLocation(m_programID, "material.kAmbient");
	m_parameters[U_MATERIAL_DIFFUSE] = glGetUniformLocation(m_programID, "material.kDiffuse");
	m_parameters[U_MATERIAL_SPECULAR] = glGetUniformLocation(m_programID, "material.kSpecular");
	m_parameters[U_MATERIAL_SHININESS] = glGetUniformLocation(m_programID, "material.kShininess");
	m_parameters[U_LIGHT0_TYPE] = glGetUniformLocation(m_programID, "lights[0].type");
	m_parameters[U_LIGHT0_POSITION] = glGetUniformLocation(m_programID, "lights[0].position_cameraspace");
	m_parameters[U_LIGHT0_COLOR] = glGetUniformLocation(m_programID, "lights[0].color");
	m_parameters[U_LIGHT0_POWER] = glGetUniformLocation(m_programID, "lights[0].power");
	m_parameters[U_LIGHT0_KC] = glGetUniformLocation(m_programID, "lights[0].kC");
	m_parameters[U_LIGHT0_KL] = glGetUniformLocation(m_programID, "lights[0].kL");
	m_parameters[U_LIGHT0_KQ] = glGetUniformLocation(m_programID, "lights[0].kQ");
	m_parameters[U_LIGHT0_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[0].spotDirection");
	m_parameters[U_LIGHT0_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[0].cosCutoff");
	m_parameters[U_LIGHT0_COSINNER] = glGetUniformLocation(m_programID, "lights[0].cosInner");
	m_parameters[U_LIGHT0_EXPONENT] = glGetUniformLocation(m_programID, "lights[0].exponent");
	m_parameters[U_COLOR_TEXTURE_ENABLED] = glGetUniformLocation(m_programID, "colorTextureEnabled");
	m_parameters[U_COLOR_TEXTURE] = glGetUniformLocation(m_programID, "colorTexture");
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
	m_parameters[U_NUMLIGHTS] = glGetUniformLocation(m_programID, "numLights");

	Mesh::SetMaterialLoc(m_parameters[U_MATERIAL_AMBIENT], m_parameters[U_MATERIAL_DIFFUSE], m_parameters[U_MATERIAL_SPECULAR], m_parameters[U_MATERIAL_SHININESS]);

	// Initialise camera properties
	//camera.Init(45.f, 45.f, 10.f);

	camera1.Init(
		glm::vec3(0.f, 3.f, 10.f),
		glm::vec3(0.f, 3.f, 0.f),
		glm::vec3(0.f, 1.f, 0.f)
	);

	camera2.Init(
		glm::vec3(10.f, 3.f, 10.f),
		glm::vec3(0.f, 3.f, 0.f),
		glm::vec3(0.f, 1.f, 0.f)
	);


	// enforce minimum Y at launch
	if (camera1.position.y < 3.3f) {
		camera1.position.y = 3.3f;
		// keep the camera looking at the same relative height (adjust target to avoid looking too far down)
		if (camera1.target.y < 3.3f) camera1.target.y = 3.3f;
		// re-initialize so FPCamera::Init() recomputes its internal vectors (Refresh)
		camera1.Init(camera1.position, camera1.target, camera1.up);
	}

	// enforce minimum Y at launch
	if (camera2.position.y < 3.3f) {
		camera2.position.y = 3.3f;
		// keep the camera looking at the same relative height (adjust target to avoid looking too far down)
		if (camera2.target.y < 3.3f) camera2.target.y = 3.3f;
		// re-initialize so FPCamera::Init() recomputes its internal vectors (Refresh)
		camera2.Init(camera2.position, camera2.target, camera2.up);
	}

	// Init VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = nullptr;
	}

	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
	meshList[GEO_SPHERE] = MeshBuilder::GenerateSphere("Sun", glm::vec3(1.f, 1.f, 1.f), 1.f, 16, 16);
	//meshList[GEO_CUBE] = MeshBuilder::GenerateCube("Arm", glm::vec3(0.5f, 0.5f, 0.5f), 1.f);

	meshList[GEO_LEFT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_LEFT]->textureID = LoadTGA("Images//blackblack.tga");
	//meshList[GEO_LEFT]->textureID = LoadTGA("Images//whitesky//whiteskyleft.tga");
	meshList[GEO_LEFT]->textureID = LoadTGA("Images//redleft copy.tga");
	//meshList[GEO_LEFT]->textureID = LoadTGA("Images//left.tga");

	meshList[GEO_RIGHT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_RIGHT]->textureID = LoadTGA("Images//blackblack.tga");
	//meshList[GEO_RIGHT]->textureID = LoadTGA("Images//whitesky//whiteskyright.tga");
	meshList[GEO_RIGHT]->textureID = LoadTGA("Images//redright copy.tga");
	//meshList[GEO_RIGHT]->textureID = LoadTGA("Images//right.tga");

	meshList[GEO_BACK] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_BACK]->textureID = LoadTGA("Images//blackblack.tga");
	//meshList[GEO_BACK]->textureID = LoadTGA("Images//whitesky//whiteskyback.tga");
	meshList[GEO_BACK]->textureID = LoadTGA("Images//redback copy.tga");
	//meshList[GEO_BACK]->textureID = LoadTGA("Images//back.tga");

	meshList[GEO_FRONT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_FRONT]->textureID = LoadTGA("Images//blackblack.tga");
	//meshList[GEO_FRONT]->textureID = LoadTGA("Images//whitesky//whiteskyfront.tga");
	meshList[GEO_FRONT]->textureID = LoadTGA("Images//redfront copy.tga");
	//meshList[GEO_FRONT]->textureID = LoadTGA("Images//front.tga");

	meshList[GEO_TOP] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//saharatop.tga");
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//top.tga");
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//whitesky//whiteskytop.tga");
	meshList[GEO_TOP]->textureID = LoadTGA("Images//redtop copy.tga");
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//bigblackmoon.tga");

	meshList[GEO_BOTTOM] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//blackblack.tga");
	//meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//whitesky//whiteskybottom.tga");
	meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//redbottom copy.tga");
	//meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//bottom.tga");

	//meshList[GEO_QUAD]->textureID = LoadTGA("Images//NYP.tga");
	meshList[GEO_QUAD] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);

	meshList[GEO_CYLINDER] = MeshBuilder::GenerateCylinder("Cylinder", glm::vec3(1.f, 1.f, 1.f), 36, 1.f, 2.f);

	meshList[GEO_GRASS] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_GRASS]->textureID = LoadTGA("Images//coast_sand_rocks_02 copy.tga");

	//meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	//meshList[GEO_GUI]->textureID = LoadTGA("Images//color.tga");

	// 16 x 16 is the number of columns and rows for the text
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//Georgia.tga");

	meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_GUI]->textureID = LoadTGA("Images//blackblack.tga");

	meshList[GEO_SHADOW] = MeshBuilder::GenerateOBJMTL("shadow", "Models//swamp_monster.obj", "Models//swamp_monster.mtl");
	meshList[GEO_SHADOW]->textureID = LoadTGA("Images//FOREST_MONSTER_V1_mat_diffuse.tga");

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);


	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(camera1.position.x, camera1.position.y, camera1.position.z);
	light[0].color = glm::vec3(1, 1, 0.5);
	light[0].type = Light::LIGHT_POINT;
	light[0].power = 0;
	light[0].kC = 1.f;
	light[0].kL = 0.01f;
	light[0].kQ = 0.001f;
	light[0].cosCutoff = 4.f;
	light[0].cosInner = 30.f;
	light[0].exponent = 3.f;
	light[0].spotDirection = glm::vec3(0.f, 1.f, 0.f);

	glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, &light[0].color.r);
	glUniform1i(m_parameters[U_LIGHT0_TYPE], light[0].type);
	glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	glUniform1f(m_parameters[U_LIGHT0_KC], light[0].kC);
	glUniform1f(m_parameters[U_LIGHT0_KL], light[0].kL);
	glUniform1f(m_parameters[U_LIGHT0_KQ], light[0].kQ);
	glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF], cosf(glm::radians<float>(light[0].cosCutoff)));
	glUniform1f(m_parameters[U_LIGHT0_COSINNER], cosf(glm::radians<float>(light[0].cosInner)));
	glUniform1f(m_parameters[U_LIGHT0_EXPONENT], light[0].exponent);

	enableLight = true;

	m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
	m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void Scene01::HandleMouseInput(FPCamera& cam) {

	double mouseX = MouseController::GetInstance()->GetMousePositionX();
	double mouseY = MouseController::GetInstance()->GetMousePositionY();

	// Skip first frame to avoid large delta
	if (firstMouse) {
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		firstMouse = false;
		return;
	}

	// Calculate mouse movement delta
	double deltaX = mouseX - lastMouseX;
	double deltaY = lastMouseY - mouseY;  // Reversed: y-coordinates go from bottom to top

	lastMouseX = mouseX;
	lastMouseY = mouseY;

	// Apply sensitivity
	deltaX *= mouseSensitivity;
	deltaY *= mouseSensitivity;

	// Update camera rotation based on mouse movement
	// This depends on your AltAzCamera implementation
	// Typical approach:
	cam.azimuth += static_cast<float>(deltaX);
	cam.altitude += static_cast<float>(deltaY);

	// Clamp altitude to prevent flipping
	if (cam.altitude > 89.0f)
		cam.altitude = 89.0f;
	if (cam.altitude < -89.0f)
		cam.altitude = -89.0f;

	// convert spherical az/alt to direction and update camera.target
	float az = glm::radians(cam.azimuth);
	float alt = glm::radians(cam.altitude);

	// spherical -> cartesian (y is up)
	glm::vec3 dir;
	dir.x = cosf(alt) * cosf(az);
	dir.y = sinf(alt);
	dir.z = cosf(alt) * sinf(az);

	cam.target = cam.position + glm::normalize(dir);

	// Re-init so FPCamera::Refresh() recalculates 'up' and other derived vectors
	// Use normalized up vector (0,1,0) instead of a scaled up vector
	cam.Init(cam.position, cam.target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Scene01::Update(double dt)
{
	HandleKeyPress1(camera1, dt);
	HandleKeyPress2(camera2, dt);

	HandleMouseInput(camera1);

	/*
	if (KeyboardController::GetInstance()->IsKeyDown('I'))
		light[0].position.z -= static_cast<float>(dt) * 5.f;
	if (KeyboardController::GetInstance()->IsKeyDown('K'))
		light[0].position.z += static_cast<float>(dt) * 5.f;
	if (KeyboardController::GetInstance()->IsKeyDown('J'))
		light[0].position.x -= static_cast<float>(dt) * 5.f;
	if (KeyboardController::GetInstance()->IsKeyDown('L'))
		light[0].position.x += static_cast<float>(dt) * 5.f;
	if (KeyboardController::GetInstance()->IsKeyDown('O'))
		light[0].position.y -= static_cast<float>(dt) * 5.f;
	if (KeyboardController::GetInstance()->IsKeyDown('P'))
		light[0].position.y += static_cast<float>(dt) * 5.f;
	*/

	// Removed calls to camera1.Update(dt) and camera2.Update(dt)
	// FPCamera::Update() processes global input (arrow keys / mouse deltas)
	// which was being applied to both cameras; input is now handled explicitly
	// in HandleKeyPress1/2 and HandleMouseInput so each camera is updated only
	// by its own handlers.

	// Prevent camera from going below ground after camera updates
	if (camera1.position.y < 3.0f) {
		camera1.position.y = 3.0f;
		if (camera1.target.y < 3.0f)
			camera1.target.y = 3.0f;
		camera1.Init(camera1.position, camera1.target, camera1.up);
	}

	if (camera2.position.y < 3.0f) {
		camera2.position.y = 3.0f;
		if (camera2.target.y < 3.0f)
			camera2.target.y = 3.0f;
		camera2.Init(camera2.position, camera2.target, camera2.up);
	}
}

void Scene01::RenderSkybox()
{
	// Front face (no rotation needed if quad faces -Z by default)
	modelStack.PushMatrix();
	modelStack.Translate(0.f, 0.f, -500.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	RenderMesh(meshList[GEO_FRONT], false);
	modelStack.PopMatrix();

	// Back face (rotate 180 degrees around Y)
	modelStack.PushMatrix();
	modelStack.Translate(0.f, 0.f, 500.f);
	modelStack.Rotate(-180.f, 1.f, 1.f, 0.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	RenderMesh(meshList[GEO_BACK], false);
	modelStack.PopMatrix();

	// Left face (rotate 90 degrees around Y)
	modelStack.PushMatrix();
	modelStack.Translate(-500.f, 0.f, 0.f);
	modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
	modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	RenderMesh(meshList[GEO_LEFT], false);
	modelStack.PopMatrix();

	// Right face (rotate -90 degrees around Y)
	modelStack.PushMatrix();
	modelStack.Translate(500.f, 0.f, 0.f);
	modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
	modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	RenderMesh(meshList[GEO_RIGHT], false);
	modelStack.PopMatrix();

	// Top face (rotate -90 degrees around X)
	modelStack.PushMatrix();
	modelStack.Translate(0.f, 500.f, 0.f);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
	modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	RenderMesh(meshList[GEO_TOP], false);
	modelStack.PopMatrix();

	// Bottom face (rotate 90 degrees around X)
	modelStack.PushMatrix();
	modelStack.Translate(0.f, -500.f, 0.f);
	modelStack.Scale(10.f, 10.f, 10.f); // CHANGE TO 10
	modelStack.Rotate(-90.f, 1.f, 0.f, 0.f);
	modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	RenderMesh(meshList[GEO_BOTTOM], false);
	modelStack.PopMatrix();
}

void Scene01::RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey)
{
	glDisable(GL_DEPTH_TEST);

	glm::mat4 ortho = glm::ortho(0.f, 800.f, 0.f, 600.f, -1000.f, 1000.f); // dimension of screen UI

	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);

	viewStack.PushMatrix();
	viewStack.LoadIdentity(); //No need camera for ortho mode

	modelStack.PushMatrix();
	modelStack.LoadIdentity();

	// To do: Use modelStack to position GUI on screen
	modelStack.Translate(x, y, 0);

	// To do: Use modelStack to scale the GUI
	modelStack.Scale(10000, 10000, 1);

	RenderMesh(mesh, false); //UI should not have light

	RenderMesh(meshList[GEO_GUI], false);

	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.PopMatrix();

	glEnable(GL_DEPTH_TEST);
}

void Scene01::RenderText(Mesh* mesh, std::string text, glm::vec3 color)
{
	if (!mesh || mesh->textureID <= 0) //Proper error check
		return;

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Disable back face culling
	glDisable(GL_CULL_FACE);

	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);

	for (unsigned i = 0; i < text.length(); ++i)
	{
		glm::mat4 characterSpacing = glm::translate(
			glm::mat4(1.f),
			glm::vec3(i * 1.f, 0, 0));
		glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));

		mesh->Render((unsigned)text[i] * 6, 6);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}


void Scene01::RenderTextOnScreen(Mesh* mesh, std::string
	text, glm::vec3 color, float size, float x, float y)
{
	if (!mesh || mesh->textureID <= 0) //Proper error check
		return;
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glm::mat4 ortho = glm::ortho(0.f, 800.f, 0.f, 600.f, -100.f, 100.f); // dimension of screen UI
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);

	viewStack.PushMatrix();
	viewStack.LoadIdentity(); //No need camera for ortho mode
	modelStack.PushMatrix();
	modelStack.LoadIdentity(); //Reset modelStack
	modelStack.Translate(x, y, 0);
	modelStack.Scale(size, size, size);

	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);

	for (unsigned i = 0; i < text.length(); ++i)
	{
		glm::mat4 characterSpacing = glm::translate(glm::mat4(1.f), glm::vec3(0.5f + i * 0.6f, 0.4f, 0));
		glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));
		mesh->Render((unsigned)text[i] * 6, 6);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);

	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.PopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void Scene01::RenderSceneFromCamera(FPCamera& cam)
{
	bool isCamera1 = (&cam == &camera1);
	bool isCamera2 = (&cam == &camera2);

	viewStack.LoadIdentity();
	viewStack.LookAt(
		cam.position.x, cam.position.y, cam.position.z,
		cam.target.x, cam.target.y, cam.target.z,
		cam.up.x, cam.up.y, cam.up.z
	);

	// Load identity matrix into the model stack
	modelStack.LoadIdentity();

	if (light[0].type == Light::LIGHT_DIRECTIONAL)
	{
		glm::vec3 lightDir(light[0].position.x, light[0].position.y, light[0].position.z);
		glm::vec3 lightDirection_cameraspace = viewStack.Top() * glm::vec4(lightDir, 0);
		glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, glm::value_ptr(lightDirection_cameraspace));
	}
	else if (light[0].type == Light::LIGHT_SPOT)
	{
		glm::vec3 lightPosition_cameraspace = viewStack.Top() * glm::vec4(light[0].position, 1);
		glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, glm::value_ptr(lightPosition_cameraspace));
		glm::vec3 spotDirection_cameraspace = viewStack.Top() * glm::vec4(light[0].spotDirection, 0);
		glUniform3fv(m_parameters[U_LIGHT0_SPOTDIRECTION], 1, glm::value_ptr(spotDirection_cameraspace));
	}
	else {
		// Calculate the light position in camera space
		glm::vec3 lightPosition_cameraspace = viewStack.Top() * glm::vec4(light[0].position, 1);
		glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, glm::value_ptr(lightPosition_cameraspace));
	}

	// ---- RENDER EVERYTHING BELOW ----

	// Render light sphere - isolated transformations
	modelStack.PushMatrix();
	modelStack.Translate(cam.position.x, 15.f, cam.position.z);
	modelStack.Scale(0.1f, 0.1f, 0.1f);
	meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
	meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_SPHERE]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_SPHERE], true);
	modelStack.PopMatrix();

	// Skybox - now renders at world origin without accumulated transforms
	RenderSkybox();

	// grass tiled from -100 to 100 on X and Z, keep existing scale (5,1,5)
	modelStack.PushMatrix();
	{
		// spacing chosen to match the previous manual placement (50 units)
		const float start = -250.f;
		const float end = 250.f;
		const float step = 50.f;
		for (float x = start; x <= end; x += step)
		{
			for (float z = start; z <= end; z += step)
			{
				modelStack.PushMatrix();
				modelStack.Translate(x, 0.f, z);
				modelStack.Scale(5.f, 1.f, 5.f);
				// keep original rotations so the tile faces the same way as before
				modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				RenderMesh(meshList[GEO_GRASS], true);
				modelStack.PopMatrix();
			}
		}
		// keep the ambient material tweak from original code
		meshList[GEO_GRASS]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	}
	modelStack.PopMatrix();

	// ----- Render Player 1 Model -----
	if (!isCamera1)  // If current view is NOT camera1
	{
		modelStack.PushMatrix();
		modelStack.Translate(camera1.position.x, camera1.position.y - 0.5f, camera1.position.z);
		modelStack.Scale(0.5f, 0.5f, 0.5f);
		modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
		meshList[GEO_SHADOW]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
		meshList[GEO_SHADOW]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
		meshList[GEO_SHADOW]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
		meshList[GEO_SHADOW]->material.kShininess = 5.0f;
		RenderMesh(meshList[GEO_SHADOW], true);
		modelStack.PopMatrix();
	}

	// ----- Render Player 2 Model -----
	if (isCamera1)  // If current view IS camera1
	{
		modelStack.PushMatrix();
		modelStack.Translate(camera2.position.x, camera2.position.y - 0.5f, camera2.position.z);
		modelStack.Scale(0.5f, 0.5f, 0.5f);
		modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
		meshList[GEO_SHADOW]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
		meshList[GEO_SHADOW]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
		meshList[GEO_SHADOW]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
		meshList[GEO_SHADOW]->material.kShininess = 5.0f;
		RenderMesh(meshList[GEO_SHADOW], true);
		modelStack.PopMatrix();
	}
}


void Scene01::Render()
{
	// Clear color buffer every frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int width = 900;
	int height = 1600;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

	// LEFT SCREEN
	glViewport(0, 0, width / 2, height);
	glClear(GL_DEPTH_BUFFER_BIT);

	projectionStack.LoadMatrix(
		glm::perspective(
			glm::radians(45.f),
			(float)(width / 2) / (float)height,
			0.1f,
			1000.f
		)
	);

	RenderSceneFromCamera(camera1);

	// RIGHT SCREEN
	glViewport(width / 2, 0, width / 2, height);
	glClear(GL_DEPTH_BUFFER_BIT);

	projectionStack.LoadMatrix(
		glm::perspective(
			glm::radians(45.f),
			(float)(width / 2) / (float)height,
			0.1f,
			1000.f
		)
	);

	RenderSceneFromCamera(camera2);

	// Render objects
	//RenderMesh(meshList[GEO_AXES], false);
}

void Scene01::RenderMesh(Mesh* mesh, bool enableLight)
{
	glm::mat4 MVP, modelView, modelView_inverse_transpose;

	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));
	modelView = viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, glm::value_ptr(modelView));

	if (enableLight) {
		glUniform1i(m_parameters[U_LIGHTENABLED], 1);
		modelView_inverse_transpose = glm::inverseTranspose(modelView);
		glUniformMatrix4fv(m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE], 1, GL_FALSE, glm::value_ptr(modelView_inverse_transpose));

		glUniform3fv(m_parameters[U_MATERIAL_AMBIENT], 1, &mesh->material.kAmbient.r);
		glUniform3fv(m_parameters[U_MATERIAL_DIFFUSE], 1, &mesh->material.kDiffuse.r);
		glUniform3fv(m_parameters[U_MATERIAL_SPECULAR], 1, &mesh->material.kSpecular.r);
		glUniform1f(m_parameters[U_MATERIAL_SHININESS], mesh->material.kShininess);
	}
	else {
		glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	}

	if (mesh->textureID > 0) {
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	}
	else {
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 0);
	}

	mesh->Render();   // only once
}


void Scene01::Exit()
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

void Scene01::HandleKeyPress1(FPCamera& cam, double dt)
{

	if (KeyboardController::GetInstance()->IsKeyPressed(0x31))
	{
		// Key press to enable culling
		glEnable(GL_CULL_FACE);
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(0x32))
	{
		// Key press to disable culling
		glDisable(GL_CULL_FACE);
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(0x33))
	{
		// Key press to enable fill mode for the polygon
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //default fill mode
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(0x34))
	{
		// Key press to enable wireframe mode for the polygon
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe mode
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(VK_SPACE))
	{
		// Change to black background
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_0))
	{
		// Toggle light on or off
	/*	enableLight = !enableLight;*/

		if (light[0].power <= 0.1f)
			light[0].power = 1.f;
		else
			light[0].power = 0.1f;
		glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_TAB))
	{
		if (light[0].type == Light::LIGHT_POINT) {
			light[0].type = Light::LIGHT_DIRECTIONAL;
		}
		else if (light[0].type == Light::LIGHT_DIRECTIONAL) {
			light[0].type = Light::LIGHT_SPOT;
		}
		else {
			light[0].type = Light::LIGHT_POINT;
		}

		glUniform1i(m_parameters[U_LIGHT0_TYPE], light[0].type);
	};

	// Calculate forward and right vectors based on camera orientation
	glm::vec3 forward = glm::normalize(cam.target - cam.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, cam.up));

	// Use IsKeyDown for continuous movement while holding the key
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W))
	{
		// Move forward
		float movement = moveSpeed * static_cast<float>(dt);
		cam.position += forward * movement;
		cam.target += forward * movement;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
	{
		// Move backward
		float movement = moveSpeed * static_cast<float>(dt);
		cam.position -= forward * movement;
		cam.target -= forward * movement;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A))
	{
		// Move left (strafe)
		float movement = moveSpeed * static_cast<float>(dt);
		cam.position -= right * movement;
		cam.target -= right * movement;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D))
	{
		// Move right (strafe)
		float movement = moveSpeed * static_cast<float>(dt);
		cam.position += right * movement;
		cam.target += right * movement;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT))
	{
		// sprint at 10.f
		float movement = (1.5 * moveSpeed) * static_cast<float>(dt);
		cam.position += forward * movement;
		cam.target += forward * movement;
	}

	// Clamp camera height to adjusted limits
	if (cam.position.y < 3.3f) {
		cam.position.y = 3.3f;
		if (cam.target.y < 3.3f)
			cam.target.y = 3.3f;
	}
	if (cam.position.y > 3.5f) {
		cam.position.y = 3.5f;
		if (cam.target.y > 3.5f)
			cam.target.y = 3.5f;
	}

	// Ensure FPCamera internal vectors are recalculated after manual changes
	cam.Init(cam.position, cam.target, cam.up);
}

void Scene01::HandleKeyPress2(FPCamera& cam, double dt)
{
	// Common toggles (culling, polygon mode, background, lights)
	if (KeyboardController::GetInstance()->IsKeyPressed(0x31)) glEnable(GL_CULL_FACE);
	if (KeyboardController::GetInstance()->IsKeyPressed(0x32)) glDisable(GL_CULL_FACE);
	if (KeyboardController::GetInstance()->IsKeyPressed(0x33)) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (KeyboardController::GetInstance()->IsKeyPressed(0x34)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (KeyboardController::GetInstance()->IsKeyPressed(VK_SPACE)) glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_0))
	{
		if (light[0].power <= 0.1f) light[0].power = 1.f;
		else light[0].power = 0.1f;
		glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_TAB))
	{
		if (light[0].type == Light::LIGHT_POINT) light[0].type = Light::LIGHT_DIRECTIONAL;
		else if (light[0].type == Light::LIGHT_DIRECTIONAL) light[0].type = Light::LIGHT_SPOT;
		else light[0].type = Light::LIGHT_POINT;
		glUniform1i(m_parameters[U_LIGHT0_TYPE], light[0].type);
	}

	// --- ROTATION (azimuth / altitude) FIRST ---
	const float rotationSpeed = 90.0f; // degrees per second

	bool rotated = false;
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_I))
	{
		cam.altitude += rotationSpeed * static_cast<float>(dt);
		rotated = true;
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_K))
	{
		cam.altitude -= rotationSpeed * static_cast<float>(dt);
		rotated = true;
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_J))
	{
		cam.azimuth -= rotationSpeed * static_cast<float>(dt);
		rotated = true;
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_L))
	{
		cam.azimuth += rotationSpeed * static_cast<float>(dt);
		rotated = true;
	}

	if (rotated)
	{
		// Clamp altitude
		if (cam.altitude > 89.0f) cam.altitude = 89.0f;
		if (cam.altitude < -89.0f) cam.altitude = -89.0f;

		// Wrap azimuth cleanly into [0,360)
		cam.azimuth = fmod(cam.azimuth + 360.0f, 360.0f);

		// Recompute spherical -> cartesian direction and update target
		float az = glm::radians(cam.azimuth);
		float alt = glm::radians(cam.altitude);
		glm::vec3 dir;
		dir.x = cosf(alt) * cosf(az);
		dir.y = sinf(alt);
		dir.z = cosf(alt) * sinf(az);
		cam.target = cam.position + glm::normalize(dir);
	}

	// --- MOVEMENT based on the up-to-date orientation ---
	glm::vec3 forward = glm::normalize(cam.target - cam.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, cam.up));

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP))
	{
		float movement = moveSpeed * static_cast<float>(dt);
		cam.position += forward * movement;
		cam.target += forward * movement;
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_DOWN))
	{
		float movement = moveSpeed * static_cast<float>(dt);
		cam.position -= forward * movement;
		cam.target -= forward * movement;
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT))
	{
		float movement = moveSpeed * static_cast<float>(dt);
		cam.position -= right * movement;
		cam.target -= right * movement;
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT))
	{
		float movement = moveSpeed * static_cast<float>(dt);
		cam.position += right * movement;
		cam.target += right * movement;
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT_SHIFT))
	{
		float movement = (1.5f * moveSpeed) * static_cast<float>(dt);
		cam.position += forward * movement;
		cam.target += forward * movement;
	}

	// Preserve vertical offset when clamping Y so pitch (altitude) is not lost.
	{
		const float minY = 3.3f;
		const float maxY = 3.5f;
		float oldY = cam.position.y;
		// clamp position.y
		cam.position.y = glm::clamp(cam.position.y, minY, maxY);
		// apply same delta to target.y to keep the view direction consistent
		float deltaY = cam.position.y - oldY;
		cam.target.y += deltaY;
	}

	// After movement, keep azimuth/altitude consistent with the new forward vector
	{
		glm::vec3 newForward = glm::normalize(cam.target - cam.position);
		float altRad = asin(glm::clamp(newForward.y, -1.0f, 1.0f));
		float azRad = atan2(newForward.z, newForward.x);
		cam.altitude = glm::degrees(altRad);
		cam.azimuth = glm::degrees(azRad);
		cam.azimuth = fmod(cam.azimuth + 360.0f, 360.0f);
	}

	// Recompute FPCamera internals once at the end
	cam.Init(cam.position, cam.target, cam.up);
}