// Jayren's Scene

#include "Scene02.h"
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

Scene02::Scene02()
{
}

Scene02::~Scene02()
{
}

void Scene02::Init()
{
	fps = 0.f;
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

	camera.Init(
		glm::vec3(0.0f, 3.0f, 0.0f), // position: Y = 1.0f
		glm::vec3(1.0f, 1.0f, 0.0f), // target:  Y = 1.0f (same height -> no pitch)
		glm::vec3(0.0f, 1.0f, 0.0f)  // world up
	);
	camera.front = glm::vec3(0.f,0.f,0.f);

	// enforce minimum Y at launch
	if (camera.position.y < 3.3f) {
		camera.position.y = 3.3f;
		// keep the camera looking at the same relative height (adjust target to avoid looking too far down)
		if (camera.target.y < 3.3f) camera.target.y = 3.3f;
		// re-initialize so FPCamera::Init() recomputes its internal vectors (Refresh)
		camera.Init(camera.position, camera.target, camera.up);
	}

	// Init VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = nullptr;
	}

	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
	meshList[GEO_SPHERE] = MeshBuilder::GenerateSphere("Sun", glm::vec3(1.f, 0.f, 0.f), 1.f, 16, 16);
	//meshList[GEO_CUBE] = MeshBuilder::GenerateCube("Arm", glm::vec3(0.5f, 0.5f, 0.5f), 1.f);

	//SKYBOX
	meshList[GEO_LEFT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_LEFT]->textureID = LoadTGA("Images//DSSkybox//DSSkyLeft.tga");

	meshList[GEO_RIGHT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_RIGHT]->textureID = LoadTGA("Images//DSSkybox//DSSkyRight.tga");

	meshList[GEO_BACK] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_BACK]->textureID = LoadTGA("Images//DSSkybox//DSSkyBack.tga");

	meshList[GEO_FRONT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_FRONT]->textureID = LoadTGA("Images//DSSkybox//DSSkyFront.tga");

	meshList[GEO_TOP] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_TOP]->textureID = LoadTGA("Images//DSSkybox//DSSkyUp.tga");

	meshList[GEO_BOTTOM] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//DSSkybox//DSSkyDown.tga");

	//meshList[GEO_QUAD]->textureID = LoadTGA("Images//NYP.tga");
	meshList[GEO_QUAD] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);

	meshList[GEO_CYLINDER] = MeshBuilder::GenerateCylinder("Cylinder", glm::vec3(1.f, 1.f, 1.f), 36, 1.f, 2.f);

	meshList[GEO_GRASS] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_GRASS]->textureID = LoadTGA("Images//DSGrass.tga");

	meshList[GEO_WALL] = MeshBuilder::GenerateCube("Wall", glm::vec3(1.f, 1.f, 1.f), 1.f);

	//meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	//meshList[GEO_GUI]->textureID = LoadTGA("Images//color.tga");

	// 16 x 16 is the number of columns and rows for the text
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//Georgia.tga");

	meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_GUI]->textureID = LoadTGA("Images//blackblack.tga");

	meshList[GEO_BLASTER] = MeshBuilder::GenerateOBJ("Blaster", "Models//DuckShoot//Blaster.obj");
	meshList[GEO_BLASTER]->textureID = LoadTGA("Images///Blaster.tga");

	meshList[GEO_DUCKTARGET] = MeshBuilder::GenerateOBJ("Target", "Models//DuckShoot//DuckTarget.obj");
	meshList[GEO_DUCKTARGET]->textureID = LoadTGA("Images//DuckTarget.tga");

	meshList[GEO_PAPER] = MeshBuilder::GenerateOBJ("Target", "Models//DuckShoot//DuckTargetInvalid.obj");
	meshList[GEO_PAPER]->textureID = LoadTGA("Images//DuckTarget.tga");

	meshList[GEO_FENCE1] = MeshBuilder::GenerateOBJ("Fence1", "Models//DuckShoot//Fence1.obj");
	meshList[GEO_FENCE1]->textureID = LoadTGA("Images//DSFence.tga");

	meshList[GEO_FENCE2] = MeshBuilder::GenerateOBJ("Fence2", "Models//DuckShoot//Fence2.obj");
	meshList[GEO_FENCE2]->textureID = LoadTGA("Images//DSFence.tga");

	meshList[GEO_FENCE3] = MeshBuilder::GenerateOBJ("Fence3", "Models//DuckShoot//Fence3.obj");
	meshList[GEO_FENCE3]->textureID = LoadTGA("Images//DSFence.tga");

	meshList[GEO_FENCE4] = MeshBuilder::GenerateOBJ("Fence4", "Models//DuckShoot//Fence4.obj");
	meshList[GEO_FENCE4]->textureID = LoadTGA("Images//DSFence.tga");

	meshList[GEO_FENCE5] = MeshBuilder::GenerateOBJ("Fence5", "Models//DuckShoot//Fence5.obj");
	meshList[GEO_FENCE5]->textureID = LoadTGA("Images//DSFence.tga");

	meshList[GEO_TENT] = MeshBuilder::GenerateOBJ("Tent", "Models//DuckShoot//Tents.obj");
	meshList[GEO_TENT]->textureID = LoadTGA("Images//DSTent.tga");

	meshList[GEO_BARREL] = MeshBuilder::GenerateOBJ("Barrel", "Models//DuckShoot//DSBarrel.obj");
	meshList[GEO_BARREL]->textureID = LoadTGA("Images//DSBarrel.tga");

	meshList[GEO_CRATE] = MeshBuilder::GenerateOBJ("Crate", "Models//DuckShoot//DSCrate.obj");
	meshList[GEO_CRATE]->textureID = LoadTGA("Images//DSCrate.tga");

	meshList[GEO_CRATES1] = MeshBuilder::GenerateOBJ("Crates1", "Models//DuckShoot//Tent1Crates1.obj");
	meshList[GEO_CRATES1]->textureID = LoadTGA("Images//DSCrate.tga");

	meshList[GEO_CRATES2] = MeshBuilder::GenerateOBJ("Crates1", "Models//DuckShoot//Tent1Crates2.obj");
	meshList[GEO_CRATES2]->textureID = LoadTGA("Images//DSCrate.tga");

	meshList[GEO_CRATES3] = MeshBuilder::GenerateOBJ("Crates1", "Models//DuckShoot//Tent1Crates3.obj");
	meshList[GEO_CRATES3]->textureID = LoadTGA("Images//DSCrate.tga");

	meshList[GEO_BARRELS1] = MeshBuilder::GenerateOBJ("Barrels1", "Models//DuckShoot//Tent1Barrels1.obj");
	meshList[GEO_BARRELS1]->textureID = LoadTGA("Images//DSBarrel.tga");

	meshList[GEO_BARRELS2] = MeshBuilder::GenerateOBJ("Barrels1", "Models//DuckShoot//Tent1Barrels2.obj");
	meshList[GEO_BARRELS2]->textureID = LoadTGA("Images//DSBarrel.tga");

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);


	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(camera.position.x, camera.position.y, camera.position.z);
	light[0].color = glm::vec3(1, 1, 0.5);
	light[0].type = Light::LIGHT_DIRECTIONAL;
	light[0].power = 1;
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
	enableHitbox = false;
	wasMousePressed = false;
	blasterAnimating = false;
	blasterMovingUp = false;
	gameEnded = false;

	blasterAngle = 0.f;
	score = 0.f;

	targetHitboxSize = glm::vec3(4.f, 5.f, 3.f);
	targetSize = glm::vec3(0.3f,0.3f,0.3f);
	targetHitboxSize *= targetSize;

	playerHitbox = PhysicsObject(2.f,2.f,2.f, glm::vec3(0.f,1.f,0.f), 0.f, 0.f, glm::vec3(0.f, 0.f, 0.f));

	m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
	m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");

	// Create Invis Walls
	{
		// Tent1 Walls
		walls.push_back(PhysicsObject(1.f, 20.f, 16.f, glm::vec3(23.f, 0.f, 30.f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 20.f, 23.48f, glm::vec3(14.43f, 0.f, 13.6f), 0.f, 0.2f, glm::vec3(0.f, 45.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 20.f, 23.48f, glm::vec3(14.43f, 0.f, 43.6f), 0.f, 0.2f, glm::vec3(0.f, -45.f, 0.f)));

		walls.push_back(PhysicsObject(1.f, 20.f, 16.f, glm::vec3(-23.f, 0.f, 30.f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 20.f, 28.5f, glm::vec3(-14.43f, 0.f, 43.6f), 0.f, 0.2f, glm::vec3(0.f, 45.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 20.f, 23.48f, glm::vec3(-14.43f, 0.f, 13.6f), 0.f, 0.2f, glm::vec3(0.f, -45.f, 0.f)));

		// Tent1 Crates
		walls.push_back(PhysicsObject(2.f, 3.786f, 13.202f, glm::vec3(0.07f, 0.f, 29.9f), 0.f, 0.5f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(2.f, 7.362f, 13.202f, glm::vec3(8.56f, 0.f, 34.8f), 0.f, 0.5f, glm::vec3(0.f, -14.2f, 0.f)));
		walls.push_back(PhysicsObject(2.f, 7.362f, 13.202f, glm::vec3(7.228f, 0.f, 20.2f), 0.f, 0.5f, glm::vec3(0.f, 26.45f, 0.f)));

		walls.push_back(PhysicsObject(2.f, 3.786f, 17.672f, glm::vec3(15.65f, 0.f, 26.3f), 0.f, 0.5f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(2.f, 7.362f, 14.916f, glm::vec3(15.65f, 0.f, 26.8f), 0.f, 0.5f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(2.f, 10.84f, 13.202f, glm::vec3(15.65f, 0.f, 26.8f), 0.f, 0.5f, glm::vec3(0.f, 0.f, 0.f)));

		// Tent2 Walls
		walls.push_back(PhysicsObject(1.f, 20.f, 16.f, glm::vec3(23.f, 0.f, 106.f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 20.f, 23.48f, glm::vec3(14.43f, 0.f, 89.6f), 0.f, 0.2f, glm::vec3(0.f, 45.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 20.f, 23.48f, glm::vec3(14.43f, 0.f, 119.6f), 0.f, 0.2f, glm::vec3(0.f, -45.f, 0.f)));

		walls.push_back(PhysicsObject(1.f, 20.f, 16.f, glm::vec3(-23.f, 0.f, 106.f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 20.f, 28.5f, glm::vec3(-14.43f, 0.f, 119.6f), 0.f, 0.2f, glm::vec3(0.f, 45.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 20.f, 23.48f, glm::vec3(-14.43f, 0.f, 89.6f), 0.f, 0.2f, glm::vec3(0.f, -45.f, 0.f)));

		// Fences
		walls.push_back(PhysicsObject(1.f, 7.85f, 14.f, glm::vec3(5.f, 0.f, -0.68f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 7.85f, 14.f, glm::vec3(-5.f, 0.f, -0.68f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));

		walls.push_back(PhysicsObject(1.f, 7.85f, 26.72f, glm::vec3(5.f, 0.f, 67.9f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 7.85f, 27.28f, glm::vec3(-5.f, 0.f, 67.6f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));

		walls.push_back(PhysicsObject(1.f, 7.85f, 21.1f, glm::vec3(5.f, 0.f, 139.44f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));
		walls.push_back(PhysicsObject(1.f, 7.85f, 21.1f, glm::vec3(-5.f, 0.f, 139.44f), 0.f, 0.2f, glm::vec3(0.f, 0.f, 0.f)));
	}

	// Test Create Targets
	/*{
		targets.push_back(new DuckTarget(glm::vec3(20.f, 10.f, -20.f), glm::vec3(-20.f, 10.f, -20.f),glm::vec3(2.f,5.f,2.f), 15.f, 5.f));

		targets.push_back(new DuckTarget(glm::vec3(20.f, 18.f, -30.f), glm::vec3(-20.f, 18.f, -30.f), glm::vec3(2.f, 5.f, 2.f), 10.f, 5.f));

		targets.push_back(new DuckTarget(glm::vec3(20.f, 26.f, -40.f), glm::vec3(-20.f, 26.f, -40.f), glm::vec3(2.f, 5.f, 2.f), 5.f, 5.f));
	}*/

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MouseController::GetInstance()->SetKeepMouseCentered(true);
}

void Scene02::HandleMouseInput(double dt) 
{

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
	camera.azimuth += static_cast<float>(deltaX);
	camera.altitude += static_cast<float>(deltaY);

	// Clamp altitude to prevent flipping
	if (camera.altitude > 89.0f)
		camera.altitude = 89.0f;
	if (camera.altitude < -89.0f)
		camera.altitude = -89.0f;

	// convert spherical az/alt to direction and update camera.target
	float az = glm::radians(camera.azimuth);
	float alt = glm::radians(camera.altitude);

	// spherical -> cartesian (y is up)
	glm::vec3 dir;
	dir.x = cosf(alt) * cosf(az);
	dir.y = sinf(alt);
	dir.z = cosf(alt) * sinf(az);

	camera.target = camera.position + glm::normalize(dir);

	// Re-init so FPCamera::Refresh() recalculates 'up' and other derived vectors
	camera.Init(camera.position, camera.target, glm::vec3(0.0f, 3.0f, 0.0f));

	bool isMousePressed = MouseController::GetInstance()->IsButtonPressed(0);

	if (isMousePressed && !wasMousePressed && !gameEnded)
	{
		PhysicsObject ball;

		ball.sizeX = 0.3f;
		ball.sizeY = 0.3f;
		ball.sizeZ = 0.3f;

		glm::vec3 forward = glm::normalize(camera.target - camera.position);
		glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
		glm::vec3 up = glm::normalize(camera.up);

		float spawnOffset = 5.0f;

		ball.pos = camera.position
			+ forward * 2.5f
			+ right * 1.0f
			- up * 1.f;

		ball.vel = forward * 100.f;

		ball.accel = glm::vec3(0, -50.f, 0);
		ball.mass = 1.f;

		projectiles.push_back(ball);

		blasterAngle = 0.f;

		blasterAnimating = true;
		blasterMovingUp = true;
	}

	// Update previous state
	wasMousePressed = isMousePressed;
}

void Scene02::Update(double dt)
{
	// Get total elapsed time from Application's m_timer
	if (!gameEnded)
	{
		totalElapsedTime += dt;
	}

	// Convert to minutes and seconds
	int minutes = static_cast<int>(totalElapsedTime) / 60;
	int seconds = static_cast<int>(totalElapsedTime) % 60;

	// Format text
	char buffer[16];
	sprintf_s(buffer, "%02d:%02d", minutes, seconds);
	elapsedTimeText = std::string(buffer);

	if (totalElapsedTime >= 300.f && !gameEnded)
	{
		gameEnded = true;
	}

	float temp = 1.f / dt;
	fps = glm::round(temp * 100.f) / 100.f;

	HandleKeyPress(dt);

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

	bool isKeyPressed = KeyboardController::GetInstance()->IsKeyDown('B');
	if (isKeyPressed && !wasKeyPressed)
	{
		glm::vec3 startPos = glm::vec3(1.f, 2.2f,24.3f);
		glm::vec3 endPos = glm::vec3(1.f, 2.2f, 35.5f);
		float speed = 3.f;
		int repeats = 2;

		SpawnTarget(startPos, endPos, targetHitboxSize, speed, repeats, 1);

		startPos = glm::vec3(5.692f, 4.f, 14.2f);
		endPos = glm::vec3(10.93f, 4.f, 25.f);
		speed = 5.f;
		repeats = 5;

		SpawnTarget(startPos, endPos, targetHitboxSize, speed, repeats, 2);

		startPos = glm::vec3(8.185f, 4.f, 40.8f);
		endPos = glm::vec3(10.93f, 4.f, 30.f);
		speed = 5.f;
		repeats = 5;

		SpawnTarget(startPos, endPos, targetHitboxSize, speed, repeats, -5);

		startPos = glm::vec3(16.72f, 5.616f, 21.1f);
		endPos = glm::vec3(16.72f, 5.616f, 32.9f);
		speed = 5.f;
		repeats = 5;

		SpawnTarget(startPos, endPos, targetHitboxSize, speed, repeats, -5);
	}

	wasKeyPressed = isKeyPressed;

	camera.Update(dt);

	glm::vec3 oldPos = playerHitbox.pos;

	// Move player
	playerHitbox.pos += playerHitbox.vel * static_cast<float>(dt);

	// Check collisions against walls
	for (int j = 0; j < walls.size(); j++) {
		PhysicsObject& wall = walls[j];
		CollisionData cd;
		if (OverlapSphere2OBB(playerHitbox, wall, cd))
		{
			playerHitbox.pos = oldPos;
			break;
		}
	}

	// Keep camera height consistent
	camera.position = glm::vec3(playerHitbox.pos.x, 3.3f, playerHitbox.pos.z);
	if (camera.position.y < 3.3f) {
		camera.position.y = 3.3f;
		if (camera.target.y < 3.3f)
			camera.target.y = 3.3f;
		camera.Init(camera.position, camera.target, camera.up);
	}

	HandleMouseInput(dt);

	for (int i = 0; i < projectiles.size(); i++) {
		PhysicsObject& ball = projectiles[i];

		for (int j = 0; j < walls.size(); j++) {
			PhysicsObject& wall = walls[j];
			CollisionData cd;
			if (OverlapSphere2OBB(ball, wall, cd))
			{
				ResolveCollision(cd);
			}
		}

		for (int j = 0; j < targets.size(); j++) {
			CollisionData cd;
			if (OverlapSphere2OBB(ball, *targets[j], cd))
			{
				if (targets[j]->OnHit())
				{
					int points = targets[j]->GetScoreValue();
					score += points;
				}
				ResolveCollision(cd);
			}
		}


		ball.UpdatePhysics(dt);


		if (ball.pos.y < -10.f)
		{
			projectiles.erase(projectiles.begin() + i);
			i--;
		}
	}

	for (int i = 0; i < targets.size(); i++)
	{
		if (targets[i]->IsActive())
		{
			targets[i]->Update(dt);
		}

		if (!targets[i]->IsActive() || targets[i]->pos.y < -10.f)
		{
			delete targets[i];
			targets.erase(targets.begin() + i);
			i--;
		}
	}

	if (blasterAnimating == true)
	{
		if (blasterMovingUp)
		{
			if (blasterAngle < 15.f)
			{
				blasterAngle += 100.f * dt;
			}
			else {
				blasterAngle = 15.f;
				blasterMovingUp = false;
			}
		}

		else {
			if (blasterAngle > 0.f)
			{
				blasterAngle -= 100.f * dt;
			}
			else {
				blasterAngle = 0.f;
				blasterAnimating = false;
			}
		}
	}

	playerHitbox.UpdatePhysics(dt);
}

void Scene02::RenderSkybox() {
	int scaleValue = 10.f;

	modelStack.PushMatrix();
	// Offset in Z direction by -50 units
	modelStack.Translate(0.f, 0.f, -50.f * scaleValue);
	modelStack.Rotate(-90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(scaleValue, scaleValue, scaleValue);
	meshList[GEO_FRONT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_FRONT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	// Offset in Z direction by -50 units
	modelStack.Translate(0.f, 0.f, 50.f * scaleValue);
	modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
	modelStack.Rotate(-90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(scaleValue, scaleValue, scaleValue);
	meshList[GEO_BACK]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_BACK], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(-50.f * scaleValue, 0.f, 0.f);
	modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
	modelStack.Rotate(-90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(scaleValue, scaleValue, scaleValue);
	meshList[GEO_LEFT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_LEFT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(50.f * scaleValue, 0.f, 0.f);
	modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
	modelStack.Rotate(-90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(scaleValue, scaleValue, scaleValue);
	meshList[GEO_RIGHT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_RIGHT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(0.f, 50.f * scaleValue, 0.f);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);

	modelStack.Scale(scaleValue, scaleValue, scaleValue);
	meshList[GEO_TOP]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_TOP], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(0.f, -50.f * scaleValue, 0.f);
	modelStack.Rotate(90.f, -1.f, 0.f, 0.f);
	modelStack.Scale(scaleValue, scaleValue, scaleValue);
	meshList[GEO_BOTTOM]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_BOTTOM], false);
	modelStack.PopMatrix();
}

void Scene02::RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey)
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

void Scene02::RenderText(Mesh* mesh, std::string text, glm::vec3 color)
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


void Scene02::RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y)
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

void Scene02::SpawnTarget(glm::vec3 startingPosition, glm::vec3 endingPosition, glm::vec3 size, float speed, int repeats, int value)
{
	targets.push_back(new DuckTarget(startingPosition, endingPosition, size, speed, repeats, value, glm::vec3(0.f,0.f,0.f)));
}

void Scene02::Render()
{
	// Clear color buffer every frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Load view matrix stack and set it with camera position, target position and up direction
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
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

	// Render objects
	//RenderMesh(meshList[GEO_AXES], false);

	// Render light sphere - isolated transformations
	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, 15.f, camera.position.z);
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
		const float start = -400.f;
		const float end = 400.f;
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

	// Materials
	{
		meshList[GEO_BLASTER]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_BLASTER]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_BLASTER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_BLASTER]->material.kShininess = 5.0f;

		meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.6f, 0.6f, 0.6f);
		meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_SPHERE]->material.kShininess = 5.0f;

		meshList[GEO_WALL]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
		meshList[GEO_WALL]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_WALL]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_WALL]->material.kShininess = 5.0f;

		meshList[GEO_DUCKTARGET]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
		meshList[GEO_DUCKTARGET]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_DUCKTARGET]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_DUCKTARGET]->material.kShininess = 5.0f;

		meshList[GEO_PAPER]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_PAPER]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_PAPER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_PAPER]->material.kShininess = 5.0f;

		meshList[GEO_FENCE1]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE1]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE1]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_FENCE1]->material.kShininess = 5.0f;

		meshList[GEO_FENCE2]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE2]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE2]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_FENCE2]->material.kShininess = 5.0f;

		meshList[GEO_FENCE3]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE3]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE3]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_FENCE3]->material.kShininess = 5.0f;

		meshList[GEO_FENCE4]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE4]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE4]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_FENCE4]->material.kShininess = 5.0f;

		meshList[GEO_FENCE5]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE5]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_FENCE5]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_FENCE5]->material.kShininess = 5.0f;

		meshList[GEO_TENT]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_TENT]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_TENT]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_TENT]->material.kShininess = 5.0f;

		meshList[GEO_BARREL]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_BARREL]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_BARREL]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_BARREL]->material.kShininess = 5.0f;

		meshList[GEO_CRATE]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_CRATE]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_CRATE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_CRATE]->material.kShininess = 5.0f;

		meshList[GEO_CRATES1]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_CRATES1]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_CRATES1]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_CRATES1]->material.kShininess = 5.0f;

		meshList[GEO_CRATES2]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_CRATES2]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_CRATES2]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_CRATES2]->material.kShininess = 5.0f;

		meshList[GEO_CRATES3]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_CRATES3]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_CRATES3]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_CRATES3]->material.kShininess = 5.0f;

		meshList[GEO_BARRELS1]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_BARRELS1]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_BARRELS1]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_BARRELS1]->material.kShininess = 5.0f;

		meshList[GEO_BARRELS2]->material.kAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_BARRELS2]->material.kDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		meshList[GEO_BARRELS2]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_BARRELS2]->material.kShininess = 5.0f;
	}

	// Environment
	{
		// Tents
		{
			// Tent1
			{
				modelStack.PushMatrix();
				modelStack.Translate(0.f, 0.f, 0.f);
				modelStack.Scale(1.f, 1.f, 1.f);
				RenderMesh(meshList[GEO_TENT], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(0.f, 0.f, 0.f);
				modelStack.Scale(1.f, 1.f, 1.f);
				RenderMesh(meshList[GEO_CRATES1], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(0.f, 0.f, 0.f);
				modelStack.Scale(1.f, 1.f, 1.f);
				RenderMesh(meshList[GEO_CRATES2], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(0.f, 0.f, 0.f);
				modelStack.Scale(1.f, 1.f, 1.f);
				RenderMesh(meshList[GEO_CRATES3], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(0.f, 0.f, 0.f);
				modelStack.Scale(1.f, 1.f, 1.f);
				RenderMesh(meshList[GEO_BARRELS1], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(0.f, 0.f, 0.f);
				modelStack.Scale(1.f, 1.f, 1.f);
				RenderMesh(meshList[GEO_BARRELS2], true);
				modelStack.PopMatrix();
			}

			// Tent2
			{

			}
		}

		// Fences
		{
			modelStack.PushMatrix();
			modelStack.Translate(0.f, 0.f, 0.f);
			modelStack.Scale(1.f, 1.f, 1.f);
			RenderMesh(meshList[GEO_FENCE1], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(0.f, 0.f, 0.f);
			modelStack.Scale(1.f, 1.f, 1.f);
			RenderMesh(meshList[GEO_FENCE2], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(0.f, 0.f, 0.f);
			modelStack.Scale(1.f, 1.f, 1.f);
			RenderMesh(meshList[GEO_FENCE3], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(0.f, 0.f, 0.f);
			modelStack.Scale(1.f, 1.f, 1.f);
			RenderMesh(meshList[GEO_FENCE4], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(0.f, 0.f, 0.f);
			modelStack.Scale(1.f, 1.f, 1.f);
			RenderMesh(meshList[GEO_FENCE5], true);
			modelStack.PopMatrix();
		}

		// Trees
		{

		}

		// Other props
		{
			/*modelStack.PushMatrix();
			modelStack.Translate(0.f, 0.f, 0.f);
			modelStack.Scale(0.3f, 0.35f, 0.3f);
			RenderMesh(meshList[GEO_BARREL], true);
			modelStack.PopMatrix();*/

			/*modelStack.PushMatrix();
			modelStack.Translate(0.f, 0.f, 0.f);
			modelStack.Scale(1.f, 1.f, 1.f);
			RenderMesh(meshList[GEO_CRATE], true);
			modelStack.PopMatrix();*/
		}
	}

	// Runtime Renders
	{
		// Render Projectiles
		for (int i = 0; i < projectiles.size(); i++) {
			PhysicsObject& ball = projectiles[i];
			modelStack.PushMatrix();
			modelStack.Translate(ball.pos.x, ball.pos.y, ball.pos.z);
			modelStack.Scale(ball.sizeX, ball.sizeY, ball.sizeZ);
			RenderMesh(meshList[GEO_SPHERE], true);
			modelStack.PopMatrix();
		}

		// Render Walls

		if (enableHitbox) {
			for (int i = 0; i < walls.size(); i++) {
				PhysicsObject& wall = walls[i];
				modelStack.PushMatrix();
				modelStack.Translate(wall.pos.x, wall.pos.y, wall.pos.z);
				modelStack.Rotate(wall.rotation.x, 1.f, 0.f, 0.f);
				modelStack.Rotate(wall.rotation.y, 0.f, 1.f, 0.f);
				modelStack.Rotate(wall.rotation.z, 0.f, 0.f, 1.f);
				modelStack.Scale(wall.sizeX, wall.sizeY, wall.sizeZ);
				RenderMesh(meshList[GEO_WALL], false);
				modelStack.PopMatrix();
			}

			modelStack.PushMatrix();
			modelStack.Translate(playerHitbox.pos.x, playerHitbox.pos.y, playerHitbox.pos.z);
			modelStack.Scale(playerHitbox.sizeX, playerHitbox.sizeY, playerHitbox.sizeZ);
			RenderMesh(meshList[GEO_SPHERE], false);
			modelStack.PopMatrix();
		}

		// Render Targets
		for (int i = 0; i < targets.size(); i++) {
			DuckTarget* target = targets[i];

			modelStack.PushMatrix();
			modelStack.Translate(target->pos.x, target->pos.y, target->pos.z);

			float yaw = target->GetRotation();
			modelStack.Rotate(yaw, 0.f, 1.f, 0.f);

			// Render main duck target mesh
			modelStack.PushMatrix();
			modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
			modelStack.Scale(targetSize.x, targetSize.y, targetSize.z);
			RenderMesh(meshList[GEO_DUCKTARGET], true);
			modelStack.PopMatrix();

			if (target->GetScoreValue() < 0)
			{
				modelStack.PushMatrix();
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				modelStack.Scale(targetSize.x, targetSize.y, targetSize.z);
				RenderMesh(meshList[GEO_PAPER], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
				modelStack.Scale(targetSize.x, targetSize.y, targetSize.z);
				RenderMesh(meshList[GEO_PAPER], true);
				modelStack.PopMatrix();
			}

			// Render hitbox if enabled
			if (enableHitbox) {
				modelStack.PushMatrix();
				modelStack.Scale(target->sizeX, target->sizeY, target->sizeZ);
				RenderMesh(meshList[GEO_WALL], false);
				modelStack.PopMatrix();
			}

			modelStack.PopMatrix();
		}
	}

	// Blaster
	{
		glm::vec3 forward = glm::normalize(camera.target - camera.position);
		glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
		glm::vec3 up = glm::normalize(camera.up);

		glm::vec3 blasterPos =
			camera.position
			+ forward * 2.f
			+ right * 1.f
			- up * 1.f;

		glm::mat4 rot(1.0f);
		rot[0] = glm::vec4(right, 0.f);
		rot[1] = glm::vec4(up, 0.f);
		rot[2] = glm::vec4(-forward, 0.f);

		modelStack.PushMatrix();
		modelStack.Translate(blasterPos.x, blasterPos.y, blasterPos.z);

		modelStack.MultMatrix(rot);
		modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(-blasterAngle, 1.f, 0.f, 0.f);
		modelStack.Scale(0.4f, 0.5f, 0.5f);

		RenderMesh(meshList[GEO_BLASTER], true);
		modelStack.PopMatrix();
	}

	// UI
	{
		// FPS COUNTER
		{
			std::string temp("FPS:" + std::to_string(fps));
			RenderTextOnScreen(meshList[GEO_TEXT], temp.substr(0, 9), glm::vec3(0, 1, 0), 20, 0, 580);
		}

		// SCORE COUNTER
		{
			std::string temp("Score:" + std::to_string(score));
			RenderTextOnScreen(meshList[GEO_TEXT], temp.substr(0,9), glm::vec3(1, 0, 0), 20, 0, 540);
		}

		// TIME COUNTER
		{
			RenderTextOnScreen(meshList[GEO_TEXT], "Time: " + elapsedTimeText, glm::vec3(1.f, 1.f, 0.f), 20, 0, 500);
		}
	}
}

void Scene02::RenderMesh(Mesh* mesh, bool enableLight)
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


void Scene02::Exit()
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

	for (int i = 0; i < targets.size(); i++)
	{
		delete targets[i];
		targets.erase(targets.begin() + i);
		i--;
	}
}

void Scene02::HandleKeyPress(double dt)
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

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_H))
	{
		enableHitbox = !enableHitbox;
	}

	// Calculate forward and right vectors based on camera orientation
	glm::vec3 forward = glm::normalize(glm::vec3(camera.target.x - camera.position.x, 0.f, camera.target.z - camera.position.z));
	glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));


	if (!gameEnded)
	{
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
			moveSpeed = 5.f * 1.5f;
		}
		else {
			moveSpeed = 5.f;
		}

		// Build requested movement delta from input (frame-rate independent)
		glm::vec3 requestedDelta(0.0f);
		float movement = moveSpeed * static_cast<float>(dt);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W))
		{
			requestedDelta += forward * movement;
		}
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
		{
			requestedDelta -= forward * movement;
		}
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A))
		{
			requestedDelta -= right * movement;
		}
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D))
		{
			requestedDelta += right * movement;
		}

		// If there is movement requested, test collisions using a swept/simple discrete test.
		if (glm::length(requestedDelta) > 1e-6f)
		{
			// Discrete test: try full move first
			PhysicsObject test = playerHitbox;
			test.pos += requestedDelta;

			bool collision = false;
			for (int i = 0; i < walls.size(); ++i)
			{
				CollisionData cd;
				if (OverlapSphere2OBB(test, walls[i], cd))
				{
					collision = true;
					break;
				}
			}

			if (!collision)
			{
				// Accept full move
				playerHitbox.pos = test.pos;
				camera.position += requestedDelta;
				camera.target += requestedDelta;
			}
			else
			{
				// Try axis-separated movement to allow sliding along walls.
				// XZ-plane only (keep Y unchanged).
				glm::vec3 deltaX = glm::vec3(requestedDelta.x, 0.0f, 0.0f);
				glm::vec3 deltaZ = glm::vec3(0.0f, 0.0f, requestedDelta.z);

				bool movedX = false;
				bool movedZ = false;

				if (glm::length(deltaX) > 1e-6f)
				{
					PhysicsObject tx = playerHitbox;
					tx.pos += deltaX;
					bool colX = false;
					for (int i = 0; i < walls.size(); ++i)
					{
						CollisionData cd;
						if (OverlapSphere2OBB(tx, walls[i], cd))
						{
							colX = true;
							break;
						}
					}
					if (!colX)
					{
						playerHitbox.pos += deltaX;
						camera.position += deltaX;
						camera.target += deltaX;
						movedX = true;
					}
				}

				if (glm::length(deltaZ) > 1e-6f)
				{
					PhysicsObject tz = playerHitbox;
					tz.pos += deltaZ;
					bool colZ = false;
					for (int i = 0; i < walls.size(); ++i)
					{
						CollisionData cd;
						if (OverlapSphere2OBB(tz, walls[i], cd))
						{
							colZ = true;
							break;
						}
					}
					if (!colZ)
					{
						playerHitbox.pos += deltaZ;
						camera.position += deltaZ;
						camera.target += deltaZ;
						movedZ = true;
					}
				}

				// If neither axis allowed movement then we are blocked (no change).
			}
		}
	}
}