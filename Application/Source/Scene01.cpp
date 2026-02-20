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
#include <cmath> // for atan2, etc.

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
	meshList[GEO_LEFT]->textureID = LoadTGA("Images//whitesky//whiteskyleft.tga");
	//meshList[GEO_LEFT]->textureID = LoadTGA("Images//left.tga");

	meshList[GEO_RIGHT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_RIGHT]->textureID = LoadTGA("Images//blackblack.tga");
	meshList[GEO_RIGHT]->textureID = LoadTGA("Images//whitesky//whiteskyright.tga");
	//meshList[GEO_RIGHT]->textureID = LoadTGA("Images//right.tga");

	meshList[GEO_BACK] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_BACK]->textureID = LoadTGA("Images//blackblack.tga");
	meshList[GEO_BACK]->textureID = LoadTGA("Images//whitesky//whiteskyback.tga");
	//meshList[GEO_BACK]->textureID = LoadTGA("Images//back.tga");

	meshList[GEO_FRONT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_FRONT]->textureID = LoadTGA("Images//blackblack.tga");
	meshList[GEO_FRONT]->textureID = LoadTGA("Images//whitesky//whiteskyfront.tga");
	//meshList[GEO_FRONT]->textureID = LoadTGA("Images//front.tga");

	meshList[GEO_TOP] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//saharatop.tga");
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//top.tga");
	meshList[GEO_TOP]->textureID = LoadTGA("Images//whitesky//whiteskytop.tga");
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//bigblackmoon.tga");

	meshList[GEO_BOTTOM] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//blackblack.tga");
	meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//whitesky//whiteskybottom.tga");
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

	meshList[GEO_ABANDONEDHOUSE] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//abandoned_house.obj", "Models//abandoned_house.mtl");
	meshList[GEO_ABANDONEDHOUSE]->textureID = LoadTGA("Images//abandonedhouseBaseColor.tga");

	meshList[BUMPERCAR] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//Bumper Car//bumper_car.obj", "Models//Bumper Car//bumper_car.mtl");
	meshList[BUMPERCAR]->textureID = LoadTGA("Images//Bumper Car//bumpercar_baseColor.tga");

	meshList[TALLTREE] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//tree//VeryTallTree.obj", "Models//tree//VeryTallTree.mtl");
	meshList[TALLTREE]->textureID = LoadTGA("Images//TallTree_baseColor.tga");

	meshList[JEFFREYEPSTEIN] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//jeffrey//jeffreyepsteinfiles.obj", "Models//jeffrey//jeffreyepsteinfiles.mtl");
	meshList[JEFFREYEPSTEIN]->textureID = LoadTGA("Images//jeffrey//jeffreyepstein_baseColor.tga");

	meshList[FOREST] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//forest//forest.obj", "Models//forest//forest.mtl");
	meshList[FOREST]->textureID = LoadTGA("Images//forest//forest_baseColor.tga");

	meshList[EXITBUTTON] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[EXITBUTTON]->textureID = LoadTGA("Images//exitScene01button.tga");

	meshList[PAUSEMENU] = MeshBuilder::GenerateQuad("pause", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[PAUSEMENU]->textureID = LoadTGA("Images//scene01pausemenuv2.tga");

	meshList[PLAYER1INDICATORUI] = MeshBuilder::GenerateQuad("player1", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[PLAYER1INDICATORUI]->textureID = LoadTGA("Images//scene01 UI//scene01player1_indicatorUI.tga");

	meshList[PLAYER2INDICATORUI] = MeshBuilder::GenerateQuad("player2", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[PLAYER2INDICATORUI]->textureID = LoadTGA("Images//scene01 UI//scene01player2_indicatorUI.tga");

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);


	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(100, 100, 0);
	light[0].color = glm::vec3(1, 1, 0.7);
	light[0].type = Light::LIGHT_DIRECTIONAL;
	light[0].power = 2;
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

	player1InCar = false;
	player2InCar = false;
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

	if (!player1InCar)
	{
		// Prevent camera from going below ground after camera updates
		if (camera1.position.y < 3.0f) {
			camera1.position.y = 3.0f;
			if (camera1.target.y < 3.0f)
				camera1.target.y = 3.0f;
			camera1.Init(camera1.position, camera1.target, camera1.up);
		}

		HandleKeyPress1(camera1, dt);
		HandleMouseInput(camera1);
	}

	if (!player2InCar)
	{
		// Prevent camera from going below ground after camera updates
		if (camera2.position.y < 3.0f) {
			camera2.position.y = 3.0f;
			if (camera2.target.y < 3.0f)
				camera2.target.y = 3.0f;
			camera2.Init(camera2.position, camera2.target, camera2.up);
		}

		HandleKeyPress2(camera2, dt);
	}

	if (player1InCar)
	{
		glm::vec3 prevPos1 = camera1.position;

		HandleKeyPress1(camera1, dt);

		HandleMouseInput(camera1);

		if (dt > 0.0)
		{
			// Cap speeds before integrating
			float s1 = glm::length(glm::vec2(cameraVelocity1.x, cameraVelocity1.z));
			if (s1 > maxSpeed) cameraVelocity1 = cameraVelocity1 * (maxSpeed / s1);
			float s2 = glm::length(glm::vec2(cameraVelocity2.x, cameraVelocity2.z));
			if (s2 > maxSpeed) cameraVelocity2 = cameraVelocity2 * (maxSpeed / s2);

			// Integrate position and target so camera moves and looks in same direction
			glm::vec3 delta1 = cameraVelocity1 * static_cast<float>(dt);
			camera1.position += delta1;
			camera1.target += delta1;

			glm::vec3 delta2 = cameraVelocity2 * static_cast<float>(dt);
			camera2.position += delta2;
			camera2.target += delta2;

			// Resolve collisions — this modifies velocities and performs positional correction
			ResolveCameraCollisionsWithBounce(camera1, cameraVelocity1, camera2, cameraVelocity2, dt);

			// Apply simple damping (friction) so cars slow after bouncing
			float damp = std::exp(-linearDamping * static_cast<float>(dt));
			cameraVelocity1 *= damp;
			cameraVelocity2 *= damp;
		}

		// Prevent camera from going below ground after camera updates
		if (camera1.position.y < 3.0f) {
			camera1.position.y = 3.0f;
			if (camera1.target.y < 3.0f)
				camera1.target.y = 3.0f;
			camera1.Init(camera1.position, camera1.target, camera1.up);
		}
	}

	if (player2InCar)
	{
		glm::vec3 prevPos2 = camera2.position;

		HandleKeyPress2(camera2, dt);

		if (dt > 0.0)
		{
			// Cap speeds before integrating
			float s1 = glm::length(glm::vec2(cameraVelocity1.x, cameraVelocity1.z));
			if (s1 > maxSpeed) cameraVelocity1 = cameraVelocity1 * (maxSpeed / s1);
			float s2 = glm::length(glm::vec2(cameraVelocity2.x, cameraVelocity2.z));
			if (s2 > maxSpeed) cameraVelocity2 = cameraVelocity2 * (maxSpeed / s2);

			// Integrate position and target so camera moves and looks in same direction
			glm::vec3 delta1 = cameraVelocity1 * static_cast<float>(dt);
			camera1.position += delta1;
			camera1.target += delta1;

			glm::vec3 delta2 = cameraVelocity2 * static_cast<float>(dt);
			camera2.position += delta2;
			camera2.target += delta2;

			// Resolve collisions — this modifies velocities and performs positional correction
			ResolveCameraCollisionsWithBounce(camera1, cameraVelocity1, camera2, cameraVelocity2, dt);

			// Apply simple damping (friction) so cars slow after bouncing
			float damp = std::exp(-linearDamping * static_cast<float>(dt));
			cameraVelocity1 *= damp;
			cameraVelocity2 *= damp;
		}

		if (camera2.position.y < 3.0f) {
			camera2.position.y = 3.0f;
			if (camera2.target.y < 3.0f)
				camera2.target.y = 3.0f;
			camera2.Init(camera2.position, camera2.target, camera2.up);
		}
	}

	float temp = 1.f / dt;
	fps = glm::round(temp * 100.f) / 100.f;
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

	glm::mat4 ortho = glm::ortho(0.f, 1600.f, 0.f, 900.f, 0.f, 100.f); // dimension of screen UI

	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);

	viewStack.PushMatrix();
	viewStack.LoadIdentity(); //No need camera for ortho mode

	modelStack.PushMatrix();
	modelStack.LoadIdentity();

	// To do: Use modelStack to position GUI on screen
	modelStack.Translate(x, y, 0);

	// To do: Use modelStack to scale the GUI
	modelStack.Scale(1600, 900, 1);

	modelStack.Rotate(90.f, 0.f, 0.f, 1.f);

	RenderMesh(mesh, false); //UI should not have light

	RenderMesh(meshList[PAUSEMENU], false);
	RenderMesh(meshList[PLAYER1INDICATORUI], false);

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
		glm::mat4 characterSpacing = glm::translate(glm::mat4(1.f), glm::vec3(0.6f + i * 0.6f, 0.4f, 0));
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
	if (!pausemenu)
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
			meshList[GEO_ABANDONEDHOUSE]->material.kDiffuse = glm::vec3(1, 1, 1);
			meshList[GEO_ABANDONEDHOUSE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
		}
		modelStack.PopMatrix();

		/*
		========================================
		FOREST
		========================================
		*/
		{

			modelStack.PushMatrix();
			modelStack.Translate(250, -2, 0.f);
			modelStack.Scale(10, 10, 10);
			meshList[FOREST]->material.kAmbient = glm::vec3(0, 0, 0);
			RenderMesh(meshList[FOREST], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(250, -2, 150);
			modelStack.Scale(10, 10, 10);
			meshList[FOREST]->material.kAmbient = glm::vec3(0, 0, 0);
			RenderMesh(meshList[FOREST], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(250, -2, -150);
			modelStack.Scale(10, 10, 10);
			meshList[FOREST]->material.kAmbient = glm::vec3(0, 0, 0);
			RenderMesh(meshList[FOREST], false);
			modelStack.PopMatrix();

		}

		{

			modelStack.PushMatrix();
			modelStack.Translate(30.f, 0.f, -10.f);
			modelStack.Scale(0.5, 0.5, 0.5);
			modelStack.Rotate(75.f, 0.f, 1.f, 0.f);
			meshList[GEO_ABANDONEDHOUSE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[GEO_ABANDONEDHOUSE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[GEO_ABANDONEDHOUSE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[GEO_ABANDONEDHOUSE]->material.kShininess = 5.0f;
			RenderMesh(meshList[GEO_ABANDONEDHOUSE], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(10.f, 0.f, -10.f);
			modelStack.Scale(0.5, 0.5, 0.5);
			modelStack.Rotate(-75.f, 0.f, 1.f, 0.f);
			meshList[GEO_ABANDONEDHOUSE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[GEO_ABANDONEDHOUSE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[GEO_ABANDONEDHOUSE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[GEO_ABANDONEDHOUSE]->material.kShininess = 5.0f;
			RenderMesh(meshList[GEO_ABANDONEDHOUSE], true);
			modelStack.PopMatrix();
		}

		modelStack.PushMatrix();
		modelStack.Translate(0.f, 0.f, -10.f);
		modelStack.Scale(0.1, 0.1, 0.1);
		meshList[TALLTREE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
		meshList[TALLTREE]->material.kDiffuse = glm::vec3(1, 1, 1);
		meshList[TALLTREE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[TALLTREE]->material.kShininess = 5.0f;
		RenderMesh(meshList[TALLTREE], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(0.f, 0.f, -25.f);
		modelStack.Scale(2, 2, 2);
		meshList[JEFFREYEPSTEIN]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
		meshList[JEFFREYEPSTEIN]->material.kDiffuse = glm::vec3(1, 1, 1);
		meshList[JEFFREYEPSTEIN]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[JEFFREYEPSTEIN]->material.kShininess = 5.0f;
		RenderMesh(meshList[JEFFREYEPSTEIN], false);
		modelStack.PopMatrix();

		if (!player1InCar)
		{
			modelStack.PushMatrix();
			modelStack.Translate(25, 0, -25);
			modelStack.Scale(3, 3, 3);
			meshList[BUMPERCAR]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
			meshList[BUMPERCAR]->material.kDiffuse = glm::vec3(1, 1, 1);
			meshList[BUMPERCAR]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
			meshList[BUMPERCAR]->material.kShininess = 5.0f;
			RenderMesh(meshList[BUMPERCAR], true);
			modelStack.PopMatrix();

			// ----- Render Player 1 Model -----
			if (!isCamera1)  // If current view is NOT camera1
			{
				modelStack.PushMatrix();
				modelStack.Translate(camera1.position.x, 0.5f, camera1.position.z);
				modelStack.Scale(2, 2, 2);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);

				// Make the model face the direction camera1 is facing:
				{
					glm::vec3 pForward = glm::normalize(camera1.target - camera1.position);
					// Yaw in degrees from X axis
					float yaw = glm::degrees(atan2(pForward.z, pForward.x));
					// Rotate the model around world Y so it faces the same horizontal direction
					modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
				}

				meshList[JEFFREYEPSTEIN]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[JEFFREYEPSTEIN]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[JEFFREYEPSTEIN]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[JEFFREYEPSTEIN]->material.kShininess = 5.0f;
				RenderMesh(meshList[JEFFREYEPSTEIN], true);
				modelStack.PopMatrix();
			}
		}

		if (!player2InCar)
		{
			modelStack.PushMatrix();
			modelStack.Translate(25, 0, -45);
			modelStack.Scale(3, 3, 3);
			meshList[BUMPERCAR]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
			meshList[BUMPERCAR]->material.kDiffuse = glm::vec3(1, 1, 1);
			meshList[BUMPERCAR]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
			meshList[BUMPERCAR]->material.kShininess = 5.0f;
			RenderMesh(meshList[BUMPERCAR], true);
			modelStack.PopMatrix();

			// ----- Render Player 2 Model -----
			if (isCamera1)  // If current view IS camera1
			{
				modelStack.PushMatrix();
				modelStack.Translate(camera2.position.x, 0.5f, camera2.position.z);
				modelStack.Scale(2, 2, 2);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);

				// Make the model face the direction camera2 is facing:
				{
					glm::vec3 pForward = glm::normalize(camera2.target - camera2.position);
					float yaw = glm::degrees(atan2(pForward.z, pForward.x));
					modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
				}

				meshList[JEFFREYEPSTEIN]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[JEFFREYEPSTEIN]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[JEFFREYEPSTEIN]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[JEFFREYEPSTEIN]->material.kShininess = 5.0f;
				RenderMesh(meshList[JEFFREYEPSTEIN], true);
				modelStack.PopMatrix();
			}
		}

		if (player1InCar)
		{
			// ----- Render Player 1 Model -----
			if (!isCamera1)  // If current view is NOT camera1
			{
				modelStack.PushMatrix();
				modelStack.Translate(camera1.position.x, 0.5f, camera1.position.z);
				modelStack.Scale(3.f, 3.f, 3.f);

				// Make the model face the direction camera1 is facing:
				{
					glm::vec3 pForward = glm::normalize(camera1.target - camera1.position);
					// Yaw in degrees from X axis
					float yaw = glm::degrees(atan2(pForward.z, pForward.x));
					// Rotate the model around world Y so it faces the same horizontal direction
					modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
				}

				meshList[BUMPERCAR]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[BUMPERCAR]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[BUMPERCAR]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[BUMPERCAR]->material.kShininess = 5.0f;
				RenderMesh(meshList[BUMPERCAR], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(camera1.position.x, 0.f, camera1.position.z);
				modelStack.Scale(2.f, 2.f, 2.f);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				// Make the model face the direction camera1 is facing:
				{
					glm::vec3 pForward = glm::normalize(camera1.target - camera1.position);
					// Yaw in degrees from X axis
					float yaw = glm::degrees(atan2(pForward.z, pForward.x));
					// Rotate the model around world Y so it faces the same horizontal direction
					modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
				}
				meshList[JEFFREYEPSTEIN]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[JEFFREYEPSTEIN]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[JEFFREYEPSTEIN]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[JEFFREYEPSTEIN]->material.kShininess = 5.0f;
				RenderMesh(meshList[JEFFREYEPSTEIN], true);
				modelStack.PopMatrix();
			}
		}
		if (player2InCar)
		{
			// ----- Render Player 2 Model -----
			if (isCamera1)  // If current view IS camera1
			{
				modelStack.PushMatrix();
				modelStack.Translate(camera2.position.x, 0.5f, camera2.position.z);
				modelStack.Scale(3.f, 3.f, 3.f);

				// Make the model face the direction camera2 is facing:
				{
					glm::vec3 pForward = glm::normalize(camera2.target - camera2.position);
					float yaw = glm::degrees(atan2(pForward.z, pForward.x));
					modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
				}

				meshList[BUMPERCAR]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[BUMPERCAR]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[BUMPERCAR]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[BUMPERCAR]->material.kShininess = 5.0f;
				RenderMesh(meshList[BUMPERCAR], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(camera2.position.x, 0.f, camera2.position.z);
				modelStack.Scale(2.f, 2.f, 2.f);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				// Make the model face the direction camera2 is facing:
				{
					glm::vec3 pForward = glm::normalize(camera2.target - camera2.position);
					float yaw = glm::degrees(atan2(pForward.z, pForward.x));
					modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
				}
				meshList[JEFFREYEPSTEIN]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[JEFFREYEPSTEIN]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[JEFFREYEPSTEIN]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[JEFFREYEPSTEIN]->material.kShininess = 5.0f;
				RenderMesh(meshList[JEFFREYEPSTEIN], true);
				modelStack.PopMatrix();
			}
		}
	}
}


void Scene01::Render()
{
	// Clear color buffer every frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!pausemenu)
	{

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

		std::string temp("FPS:" + std::to_string(fps));
		RenderTextOnScreen(meshList[GEO_TEXT], temp.substr(0, 9), glm::vec3(1, 1, 1), 25, 5, 45);

		RenderTextOnScreen(meshList[GEO_TEXT], "Z to open menu", glm::vec3(1, 1, 1), 25, 5, 15);

		/*
		modelStack.PushMatrix();
		RenderMeshOnScreen(meshList[PLAYER1INDICATORUI], 3, 5, 0.2, 0.2);
		modelStack.PopMatrix();
		*/

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

	if (pausemenu)
	{
		glViewport(0, 0, 1600, 900);
		RenderMeshOnScreen(meshList[PAUSEMENU], 800, 450, 1600, 900);
	}
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

void Scene01::ResolveCameraCollisionsWithBounce(FPCamera& a, glm::vec3& velA, FPCamera& b, glm::vec3& velB, double dt)
{
	// Work in XZ plane; preserve Y
	glm::vec2 posA(a.position.x, a.position.z);
	glm::vec2 posB(b.position.x, b.position.z);

	glm::vec2 delta = posA - posB;
	float dist = glm::length(delta);
	float minDist = cameraRadius + cameraRadius;

	// avoid zero-length
	if (dist < 1e-6f)
	{
		delta = glm::vec2(1.0f, 0.0f);
		dist = 1.0f;
	}

	if (dist < minDist)
	{
		glm::vec2 n = delta / dist; // normal from B to A

		// relative velocity along normal
		glm::vec2 relVel = glm::vec2(velA.x - velB.x, velA.z - velB.z);
		float velAlongNormal = glm::dot(relVel, n);

		// if moving towards each other, compute impulse
		if (velAlongNormal < 0.0f)
		{
			float invMassA = 1.0f / cameraMass;
			float invMassB = 1.0f / cameraMass;
			float j = -(1.0f + restitution) * velAlongNormal / (invMassA + invMassB);

			glm::vec2 impulse = j * n;
			velA.x += impulse.x * invMassA;
			velA.z += impulse.y * invMassA;
			velB.x -= impulse.x * invMassB;
			velB.z -= impulse.y * invMassB;

			PlaySound(TEXT("Sounds//fah.wav"), NULL, SND_FILENAME | SND_ASYNC);
		}

		// Positional correction to prevent sinking (split by mass)
		const float percent = 0.8f; // usually 20%..80%
		const float slop = 0.01f;   // penetration allowance
		float penetration = minDist - dist;
		float correctionMag = (glm::max)(penetration - slop, 0.0f) * percent;
		glm::vec2 correction = (correctionMag / 2.0f) * n; // split between the two

		// Save old positions to compute target shifts
		glm::vec2 oldPosA = posA;
		glm::vec2 oldPosB = posB;

		posA += correction;
		posB -= correction;

		// Commit corrected positions back (preserve Y)
		a.position.x = posA.x;
		a.position.z = posA.y;
		b.position.x = posB.x;
		b.position.z = posB.y;

		// Shift targets by same positional delta so orientation remains consistent
		glm::vec3 shiftA(posA.x - oldPosA.x, 0.0f, posA.y - oldPosA.y);
		glm::vec3 shiftB(posB.x - oldPosB.x, 0.0f, posB.y - oldPosB.y);
		a.target += shiftA;
		b.target += shiftB;
	}
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

	glm::vec3* pVel = (&cam == &camera1) ? &cameraVelocity1 : &cameraVelocity2;

	// Calculate forward and right vectors based on camera orientation
	glm::vec3 forward = glm::normalize(cam.target - cam.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, cam.up));

	if (!player1InCar)
	{
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
	}

	if (player1InCar)
	{
		// Use IsKeyDown for continuous movement while holding the key
		glm::vec3 inputAccel(0.0f);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W)) inputAccel += forward;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S)) inputAccel -= forward;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A)) inputAccel -= right;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D)) inputAccel += right;

		float accelScale = driveAcceleration;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT))
			accelScale *= 1.5f;

		if (glm::length(glm::vec2(inputAccel.x, inputAccel.z)) > 0.001f)
		{
			// normalize across XZ and apply acceleration magnitude (keep Y zero)
			glm::vec3 dir = glm::normalize(glm::vec3(inputAccel.x, 0.0f, inputAccel.z));
			(*pVel) += dir * (accelScale * static_cast<float>(dt));
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

	// pause menu
	{
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_Z))
		{
			if (!pausemenu)
			{
				pausemenu = true;
			}
			else
			{
				pausemenu = false;
			}
		}
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E))
	{
		const glm::vec3 minPos(22, -1, -27);
		const glm::vec3 maxPos(27, 5, -22);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		if (isInsideBox(cam.position, minPos, maxPos))
		{
			player1InCar = true;
		}
	}
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

	if (!player2InCar)
	{
		// 1. Use actual delta time from your engine/timer
		// float dt = Timer::GetDeltaTime(); 
		float dt = 0.016f; // Placeholder for ~60fps

		const float rotationSpeed = 50.0f; // Adjusted for real dt
		float turnAmount = rotationSpeed * dt;

		// 2. Update Angles (The Source of Truth)
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_I)) cam.altitude += turnAmount;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_K)) cam.altitude -= turnAmount;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_L)) cam.azimuth += turnAmount;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_J)) cam.azimuth -= turnAmount;

		// 3. Clamp and Wrap
		cam.altitude = glm::clamp(cam.altitude, -89.0f, 89.0f);
		cam.azimuth = fmod(cam.azimuth + 360.0f, 360.0f);

		// 4. Convert to Direction
		float az = glm::radians(cam.azimuth);
		float alt = glm::radians(cam.altitude);

		glm::vec3 dir;
		dir.x = cosf(alt) * cosf(az);
		dir.y = sinf(alt);
		dir.z = cosf(alt) * sinf(az);

		// 5. Update Camera
		cam.target = cam.position + dir; // dir is already unit length via trig

		// Use a standard Up vector to prevent tilt/roll
		cam.Init(cam.position, cam.target, glm::vec3(0.0f, 1.0f, 0.0f));

		// REMOVED: The block that re-calculates altitude/azimuth from the vector.
		// It is unnecessary and is what's causing your "snapping" to horizontal.
	}

	glm::vec3* pVel = (&cam == &camera1) ? &cameraVelocity1 : &cameraVelocity2;

	// --- MOVEMENT based on the up-to-date orientation ---
	glm::vec3 forward = glm::normalize(cam.target - cam.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, cam.up));

	if (!player2InCar)
	{
		float movement = moveSpeed * static_cast<float>(dt);

		// 1. Calculate Movement
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP))    cam.position += forward * movement;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_DOWN))  cam.position -= forward * movement;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT))  cam.position -= right * movement;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT)) cam.position += right * movement;

		// 2. Clamp only the Position
		cam.position.y = glm::clamp(cam.position.y, 3.3f, 3.5f);

		// 3. RE-CALCULATE target based on current position + stored angles
		// This prevents the "snap" to horizontal level.
		float az = glm::radians(cam.azimuth);
		float alt = glm::radians(cam.altitude);

		glm::vec3 dir;
		dir.x = cosf(alt) * cosf(az);
		dir.y = sinf(alt);
		dir.z = cosf(alt) * sinf(az);

		cam.target = cam.position + dir;

		// 4. Update internal camera vectors
		cam.Init(cam.position, cam.target, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	if (player2InCar)
	{
		// --- 1. ROTATION (Update the Master Angles) ---
		const float rotationSpeed = 90.0f;

		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_I)) cam.altitude += rotationSpeed * dt;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_K)) cam.altitude -= rotationSpeed * dt;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_J)) cam.azimuth -= rotationSpeed * dt;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_L)) cam.azimuth += rotationSpeed * dt;

		// Clamp altitude to stay horizontal-ish and wrap azimuth
		cam.altitude = glm::clamp(cam.altitude, -89.0f, 89.0f);
		cam.azimuth = fmod(cam.azimuth + 360.0f, 360.0f);

		// --- 2. MOVEMENT (Update Velocity/Position) ---
		glm::vec3 inputAccel(0.0f);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP))    inputAccel += forward;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_DOWN))  inputAccel -= forward;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT))  inputAccel -= right;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT)) inputAccel += right;

		float accelScale = driveAcceleration;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT_SHIFT)) accelScale *= 1.5f;

		if (glm::length(glm::vec2(inputAccel.x, inputAccel.z)) > 0.001f)
		{
			glm::vec3 moveDir = glm::normalize(glm::vec3(inputAccel.x, 0.0f, inputAccel.z));
			(*pVel) += moveDir * (accelScale * static_cast<float>(dt));
		}

		// --- 3. FINAL TARGET SYNC ---
		// Now that the car has potentially moved (changing cam.position), 
		// we force the target to stay at the correct angle relative to the new position.

		float azRad = glm::radians(cam.azimuth);
		float altRad = glm::radians(cam.altitude);

		glm::vec3 lookDir;
		lookDir.x = cosf(altRad) * cosf(azRad);
		lookDir.y = sinf(altRad);
		lookDir.z = cosf(altRad) * sinf(azRad);

		// This ensures the camera looks where the ANGLES say it should look,
		// regardless of how the car turned.
		cam.target = cam.position + lookDir;

		cam.Init(cam.position, cam.target, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_C))
	{
		const glm::vec3 minPos(22, -1, -47);
		const glm::vec3 maxPos(27, 5, -42);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		if (isInsideBox(cam.position, minPos, maxPos))
		{
			player2InCar = true;
		}
	}
}