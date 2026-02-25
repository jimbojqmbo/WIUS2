//Alvin
//add mouse sens adjuster?

#include "Scene03.h"
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

Scene03::Scene03()
{
}

Scene03::~Scene03()
{
}

void Scene03::Init()
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

	camera.Init(
		glm::vec3(0.0f, 1.0f, 30.0f), // was 0,1,14
		glm::vec3(0.0f, 1.0f, 0.0f), // target:  Y = 1.0f (same height -> no pitch)
		glm::vec3(0.0f, 1.0f, 0.0f)  // world up
	);

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
	meshList[GEO_SPHERE] = MeshBuilder::GenerateSphere("Sun", glm::vec3(1.f, 1.f, 1.f), 1.f, 16, 16);
	//meshList[GEO_CUBE] = MeshBuilder::GenerateCube("Arm", glm::vec3(0.5f, 0.5f, 0.5f), 1.f);

	//SKYBOX
	meshList[GEO_LEFT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_LEFT]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Left.tga");

	meshList[GEO_RIGHT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_RIGHT]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Right.tga");

	meshList[GEO_BACK] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_BACK]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Back.tga");

	meshList[GEO_FRONT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_FRONT]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Front.tga");

	meshList[GEO_TOP] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_TOP]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Top.tga");

	meshList[GEO_BOTTOM] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Bottom.tga");
	//SKYBOX

	meshList[GEO_QUAD] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);

	meshList[GEO_CYLINDER] = MeshBuilder::GenerateCylinder("Cylinder", glm::vec3(1.f, 1.f, 1.f), 36, 1.f, 2.f);

	meshList[GEO_GRASS] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_GRASS]->textureID = LoadTGA("Images/alvingrass.tga");

	// 16 x 16 is the number of columns and rows for the text
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//Georgia.tga");

	meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_GUI]->textureID = LoadTGA("Images//blackblack.tga");

	meshList[GEO_BASKETBALL] = MeshBuilder::GenerateOBJMTL("Basketball", "Models//basketball.obj", "Models//basketball.mtl");
	meshList[GEO_BASKETBALL]->textureID = LoadTGA("Images//basketball.tga");

	meshList[GEO_HOOP] = MeshBuilder::GenerateOBJMTL("hoop", "Models//hoop.obj", "Models//hoop.mtl");
	meshList[GEO_HOOP]->textureID = LoadTGA("Images//hoop.tga");

	meshList[GEO_TORUS] = MeshBuilder::GenerateTorus("Torus", glm::vec3(1, 1, 1), 0.02, 1.1);

	meshList[GEO_BLACKWALL] = MeshBuilder::GenerateCube("BlackWall", glm::vec3(0, 0, 0), 1.f);

	meshList[GEO_TENT] = MeshBuilder::GenerateOBJMTL("CarnivalTent", "Models//alvintent.obj", "Models//alvintent.mtl");
	meshList[GEO_TENT]->textureID = LoadTGA("Images//alvintent.tga");

	meshList[GEO_HOTDOG] = MeshBuilder::GenerateOBJMTL("hotdog", "Models//hotdog.obj", "Models//hotdog.mtl");
	meshList[GEO_HOTDOG]->textureID = LoadTGA("Images//hotdog.tga");

	meshList[GEO_CLOWN] = MeshBuilder::GenerateOBJMTL("Clown", "Models//clown.obj", "Models//clown.mtl");
	meshList[GEO_CLOWN]->textureID = LoadTGA("Images//clown.tga");

	meshList[GEO_BALLOON] = MeshBuilder::GenerateOBJMTL("Balloon", "Models//red_balloon.obj", "Models//red_balloon.mtl");
	meshList[GEO_BALLOON]->textureID = LoadTGA("Images//red_balloon.tga");

	meshList[GEO_BIRCHTREE] = MeshBuilder::GenerateOBJMTL("BirchTree", "Models//birchtree//birchtree.obj", "Models//birchtree//birchtree.mtl");
	meshList[GEO_BIRCHTREE]->textureID = LoadTGA("Images//birchtree//birchtree_baseColor.tga");

	meshList[GEO_CAROUSEL] = MeshBuilder::GenerateOBJMTL("Carousel", "Models//carousel.obj", "Models//carousel.mtl");
	meshList[GEO_CAROUSEL]->textureID = LoadTGA("Images//carousel.tga");

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);


	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(camera.position.x, camera.position.y, camera.position.z);
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

	startarea = glm::vec3(0.f, 1.f, 14.f);
	startarearadius = 5.f;
}

void Scene03::HandleMouseInput() {

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
	
	/*if (MouseController::GetInstance()->IsButtonPressed(0)) {
		PhysicsObject ball;
		ball.pos = camera.position;
		ball.accel.Set(0, -300.f, 0);

		Vector3 disp = camera.target - camera.position;

		if (disp.IsZero() == false) {
			disp.Normalize();
			ball.vel = disp * 200.f;
		}

		projectiles.push_back(ball);
	}*/
}

bool Scene03::OverlapCircle2AABB(glm::vec3 circlePos, float radius, glm::vec3 boxMin, glm::vec3 boxMax)
{
	glm::vec3 closest = circlePos;

	if (closest.x < boxMin.x) {
		closest.x = boxMin.x;
	}
	else if (closest.x > boxMax.x) {
		closest.x = boxMax.x;
	}

	if (closest.y < boxMin.y) {
		closest.y = boxMin.y;
	}
	else if (closest.y > boxMax.y) {
		closest.y = boxMax.y;
	}

	glm::vec3 diff = circlePos - closest;

	// Step 3: distance check
	float distSq = diff.x * diff.x + diff.y * diff.y;

	if (distSq <= radius * radius) {
		return true;
	}

	return false;
}

void Scene03::Update(double dt)
{
	// Toggle score display anytime after game ends
	if (endgame && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_C))
	{
		showScore = !showScore;
	}

	if (!showScore) {
		showCrosshair = false;
	}

	//int mouseX = MouseController::GetInstance()->GetMousePositionX();
	//int mouseY = MouseController::GetInstance()->GetMousePositionY();
	//std::cout << mouseX << ", " << mouseY << std::endl;

	//showCrosshair = true;

	// Get total elapsed time from Application's m_timer

	spinspeed += 100 * dt;

	if (timerStarted && !timerEnded)
	{
		totalElapsedTime += dt;

		if (totalElapsedTime >= timelimit)
		{
			totalElapsedTime = timelimit;
			timerEnded = true;
			std::cout << "Timer ended \n";
			showScore = true;
			endgame = true;
			restartgame = false;
		}
	}

	// Convert to minutes and seconds
	int minutes = static_cast<int>(totalElapsedTime) / 60;
	int seconds = static_cast<int>(totalElapsedTime) % 60;

	// Format text
	char buffer[16];
	sprintf_s(buffer, "%02d:%02d", minutes, seconds);
	elapsedTimeText = std::string(buffer);

	//std::cout << "Time: " << elapsedTimeText << std::endl;

	float boardMinX = hoopPosition.x - 2.2f;
	float boardMaxX = hoopPosition.x + 2.2f;

	float boardMinY = hoopPosition.y + 5.5f;
	float boardMaxY = hoopPosition.y + 8.5f;

	float boardZ = hoopPosition.z + 2.3f;

	//camera.position.y = 3.f;

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_COMMA)) {
		camera.position.y = 3.f;
	}

	//std::cout << camera.position.x << ", " << camera.position.y << ", " << camera.position.z << std::endl;

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

	camera.Update(dt);

	// Prevent camera from going below ground after camera updates
	/*if (camera.position.y < 3.0f) {
		camera.position.y = 3.0f;
		if (camera.target.y < 3.0f)
			camera.target.y = 3.0f;
		camera.Init(camera.position, camera.target, camera.up);
	}*/

	HandleMouseInput();

	/*if (spacePressed == false) {
		if (KeyboardController::GetInstance()->IsKeyPressed(VK_SPACE) && isGrounded && spacePressed == false)
		{
			spacePressed = true;
			playerYVelocity = jumpForce;
			isGrounded = false;
		}
	}

	playerYVelocity -= gravity * dt;
	camera.position.y += playerYVelocity * dt;
	camera.target.y += playerYVelocity * dt;

	if (camera.position.y <= 2.f)
	{
		camera.position.y = 2.f;
		playerYVelocity = 0.f;
		isGrounded = true;
	}

	if (isGrounded == true) {
		spacePressed == false;
	}*/

	if (OverlapCircle2Circle(camera.position, 2.f, startarea, startarearadius)) {
		inStartArea = true;
	}
	else {
		inStartArea = false;
	}

	std::cout << light->position.x << ", " <<
				 light->position.y << ", " <<
				 light->position.z << std::endl;

	if (inStartArea) {
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_R))
		{
			restartgame = true;
			endgame = false;
			timerStarted = false;
			timerEnded = false;
			totalElapsedTime = 0.0;
			pointcounter = 0;
			showScore = false;

			ballThrown = false;
			hasBall = true;
			ball.vel = glm::vec3(0.f);
			ball.accel = glm::vec3(0.f);

			hoopPosition = glm::vec3(0.f, 0.f, 0.f);
			hoopDirection = 1;

			camera.position.x = 0.f;
			camera.position.y = 3.f;
			camera.position.z = 14.f;
			camera.target = glm::vec3(0.f, 3.f, 0.f);
			camera.Init(camera.position, camera.target, glm::vec3(0.f, 1.f, 0.f));

			showstart = false;
		}
	}

	

	if (restartgame) {
		camera.position.x = 0.f;
		camera.position.y = 3.f;
		camera.position.z = 14.f;

		bool mouseCurrentlyDown = MouseController::GetInstance()->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT);

		if (mouseCurrentlyDown && !mousePreviouslyDown && hasBall)
		{
			glm::vec3 forward = glm::normalize(camera.target - camera.position);

			ball.pos = camera.position + forward * 2.0f;
			ball.vel = forward * 25.f;
			ball.vel.y = 14.f;
			ball.accel.y = -gravity;

			hasBall = false;
			ballThrown = true;

			//PlaySound(TEXT("Sounds//fah.wav"), NULL, SND_FILENAME | SND_SYNC);

			if (!timerStarted)
			{
				timerStarted = true;
			}
		}

		mousePreviouslyDown = mouseCurrentlyDown;

		if (ballThrown)
		{
			ball.UpdatePhysics(dt);

			if (ball.pos.y <= 0.4f)
			{
				ball.pos.y = 0.4f;
				ball.vel.y *= -0.7f;

				float friction = 0.98f;
				ball.vel.x *= friction;
				ball.vel.z *= friction;

				if (abs(ball.vel.x) < 0.05f) ball.vel.x = 0.f;
				if (abs(ball.vel.z) < 0.05f) ball.vel.z = 0.f;
			}

			float boardMinX = hoopPosition.x - 2.2f;
			float boardMaxX = hoopPosition.x + 2.2f;
			float boardMinY = hoopPosition.y + 5.5f;
			float boardMaxY = hoopPosition.y + 8.5f;
			float boardZ = hoopPosition.z + 2.3f;

			if (ball.pos.x >= boardMinX &&
				ball.pos.x <= boardMaxX &&
				ball.pos.y >= boardMinY &&
				ball.pos.y <= boardMaxY)
			{
				if (abs(ball.pos.z - boardZ) <= ballRadius)
				{
					float direction = (ball.pos.z > boardZ) ? 1.f : -1.f;
					ball.pos.z = boardZ + direction * ballRadius;
					ball.vel.z = -ball.vel.z * 0.4f;
				}
			}

			glm::vec3 toBall = ball.pos - rimPosition;
			glm::vec3 flat = glm::vec3(toBall.x, 0.f, toBall.z);
			float distToCenter = glm::length(flat);

			if (distToCenter > 0.0001f)
			{
				glm::vec3 closestCirclePoint =
					rimPosition + glm::normalize(flat) * rimbigR;

				float distToTube =
					glm::length(ball.pos - closestCirclePoint);

				if (distToTube <= (rimsmallR + ballRadius))
				{
					glm::vec3 normal =
						glm::normalize(ball.pos - closestCirclePoint);

					float penetration =
						(rimsmallR + ballRadius) - distToTube;

					ball.pos += normal * penetration;

					if (glm::dot(ball.vel, normal) < 0.f)
					{
						ball.vel = glm::reflect(ball.vel, normal);
						ball.vel *= 0.7f;
					}
				}
			}

			float distanceToPlayer = glm::length(ball.pos - camera.position);

			/*if (distanceToPlayer < pickupDistance && glm::length(ball.vel) < 1.0f)
			{

				if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E)) {
					ballThrown = false;
					hasBall = true;
				}
			}*/

			if (ballThrown && ball.pos.z <= 0.f)
			{
				hasBall = true;
				ballThrown = false;

				// Reset physics
				ball.vel = glm::vec3(0.f);
				ball.accel = glm::vec3(0.f);
			}

			if (ballThrown && ball.pos.y <= 2.f)
			{
				hasBall = true;
				ballThrown = false;

				// Reset physics
				ball.vel = glm::vec3(0.f);
				ball.accel = glm::vec3(0.f);
			}

			bool isOverlapping = OverlapCircle2Circle(glm::vec3(rimPosition.x, rimPosition.y - 0.5f, rimPosition.z), 0.01f, ball.pos, ballRadius);
			if (isOverlapping && !rimWasOverlapping)
			{
				pointcounter += 2;
				hoopSpeed += 0.25f;
				std::cout << "Points: " << pointcounter << std::endl;
			}

			rimWasOverlapping = isOverlapping;

			if (OverlapCircle2Circle(camera.position, pickupDistance, ball.pos, ballRadius)) {
				if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E)) {
					ballThrown = false;
					hasBall = true;
				}
			}
		}
	}

	if (restartgame == false) {
		showstart = true;
	}

	hoopPosition.x += hoopDirection * hoopSpeed * static_cast<float>(dt);

	if (hoopPosition.x > hoopLimit)
	{
		hoopPosition.x = hoopLimit;
		hoopDirection = -1;
	}
	else if (hoopPosition.x < -hoopLimit)
	{
		hoopPosition.x = -hoopLimit;
		hoopDirection = 1;
	}

	rimPosition = hoopPosition + glm::vec3(0.f, 6.37f, 3.2f);

	float temp = 1.f / dt;
	fps = glm::round(temp * 100.f) / 100.f;
}

void Scene03::RenderSkybox() {
	modelStack.PushMatrix();
	// Offset in Z direction by -50 units
	modelStack.Translate(0.f, 0.f, -50.f);
	meshList[GEO_FRONT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_FRONT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	// Offset in Z direction by -50 units
	modelStack.Translate(0.f, 0.f, 50.f);
	modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
	meshList[GEO_BACK]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_BACK], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(-50.f, 0.f, 0.f);
	modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
	meshList[GEO_LEFT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_LEFT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(50.f, 0.f, 0.f);
	modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
	meshList[GEO_RIGHT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_RIGHT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(0.f, 50.f, 0.f);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
	meshList[GEO_TOP]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_TOP], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(0.f, -50.f, 0.f);
	modelStack.Rotate(90.f, -1.f, 0.f, 0.f);
	meshList[GEO_BOTTOM]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_BOTTOM], false);
	modelStack.PopMatrix();
}

void Scene03::RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey)
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

void Scene03::RenderText(Mesh* mesh, std::string text, glm::vec3 color)
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


void Scene03::RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y)
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

void Scene03::Render()
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

	glm::vec3 forward = glm::normalize(camera.target - camera.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
	glm::vec3 up = glm::normalize(camera.up);

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
	//modelStack.PushMatrix();
	//modelStack.Translate(camera.position.x, 15.f, camera.position.z);
	//modelStack.Scale(0.1f, 0.1f, 0.1f);
	//meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	//meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
	//meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	//meshList[GEO_SPHERE]->material.kShininess = 5.0f;
	//RenderMesh(meshList[GEO_SPHERE], true);
	//modelStack.PopMatrix();

	// Skybox - now renders at world origin without accumulated transforms
	RenderSkybox();

	/*modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, camera.position.y, camera.position.z);
	glm::vec3 offset = forward * 1.5f + right * 0.7f + up * -0.6f;
	modelStack.Translate(offset.x, offset.y, offset.z);
	glm::mat4 viewRotation = viewStack.Top();
	viewRotation[3] = glm::vec4(0, 0, 0, 1);
	glm::mat4 inverseRotation = glm::inverse(viewRotation);
	modelStack.MultMatrix(inverseRotation);
	modelStack.Scale(0.008f, 0.008f, 0.008f);
	RenderMesh(meshList[GEO_BASKETBALL], false);
	modelStack.PopMatrix();*/

	/*for (auto& ball : balls)
	{
		modelStack.PushMatrix();
		modelStack.Translate(ball.pos.x, ball.pos.y, ball.pos.z);
		modelStack.Scale(0.008f, 0.008f, 0.008f);
		RenderMesh(meshList[GEO_BASKETBALL], false);
		modelStack.PopMatrix();
	}*/

	if (restartgame) {
		if (ballThrown)
		{
			modelStack.PushMatrix();
			modelStack.Translate(ball.pos.x, ball.pos.y, ball.pos.z);
			modelStack.Scale(0.008f, 0.008f, 0.008f);
			RenderMesh(meshList[GEO_BASKETBALL], false);
			modelStack.PopMatrix();
		}
		else if (hasBall)
		{
			// render in front of camera
			glm::vec3 forward = glm::normalize(camera.target - camera.position);
			glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
			glm::vec3 up = glm::normalize(camera.up);

			glm::vec3 offset = forward * 1.5f + right * 0.7f + up * -0.6f;

			modelStack.PushMatrix();
			modelStack.Translate(
				camera.position.x + offset.x,
				camera.position.y + offset.y,
				camera.position.z + offset.z
			);
			modelStack.Scale(0.008f, 0.008f, 0.008f);
			meshList[GEO_BASKETBALL]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[GEO_BASKETBALL]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
			meshList[GEO_BASKETBALL]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
			meshList[GEO_BASKETBALL]->material.kShininess = 5.0f;
			RenderMesh(meshList[GEO_BASKETBALL], true);
			modelStack.PopMatrix();
		}
	}

	modelStack.PushMatrix();
	modelStack.Translate(hoopPosition.x, hoopPosition.y, hoopPosition.z);
	modelStack.Scale(2.f, 2.f, 2.f);
	meshList[GEO_HOOP]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_HOOP]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_HOOP]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_HOOP]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_HOOP], true);
	modelStack.PopMatrix();


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
		meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
		meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_SPHERE]->material.kShininess = 5.0f;
	}
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(rimPosition.x, rimPosition.y, rimPosition.z);
	modelStack.Scale(0.5f, 0.5f, 0.5f);
	RenderMesh(meshList[GEO_TORUS], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(0.f, 4.5f, -0.5f);
	modelStack.Scale(14.f, 9.f, 3.f);
	meshList[GEO_BLACKWALL]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_BLACKWALL]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_BLACKWALL]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_BLACKWALL]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_BLACKWALL], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(25.f, 6.f, -20.f);
	modelStack.Scale(8.f, 8.f, 8.f);
	modelStack.Rotate(225.f, 0.f, 1.f, 0.f);
	meshList[GEO_TENT]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_TENT]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_TENT]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_TENT]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_TENT], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(-18.f, 0.f, 8.f);
	modelStack.Scale(1.f, 1.f, 1.f);
	modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
	meshList[GEO_HOTDOG]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_HOTDOG]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_HOTDOG]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_HOTDOG]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_HOTDOG], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(15.f, 0.f, -3.f);
	modelStack.Scale(2.f, 2.f, 2.f);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
	meshList[GEO_CLOWN]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_CLOWN]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_CLOWN]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_CLOWN]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_CLOWN], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(13.4f, 2.5f, -2.85f);
	modelStack.Scale(1.f, 1.f, 1.f);
	modelStack.Rotate(0.f, 0.f, 1.f, 0.f);
	meshList[GEO_BALLOON]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_BALLOON]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_BALLOON]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_BALLOON]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_BALLOON], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(-19, 0, 15);
	modelStack.Scale(0.03f, 0.03f, 0.03f);
	modelStack.Rotate(0.f, 0.f, 1.f, 0.f);
	meshList[GEO_BIRCHTREE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_BIRCHTREE]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_BIRCHTREE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_BIRCHTREE]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_BIRCHTREE], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(-22.5, 0, 31.5);
	modelStack.Scale(80.f, 80.f, 80.f);
	modelStack.Rotate(spinspeed, 0.f, 1.f, 0.f);
	meshList[GEO_CAROUSEL]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_CAROUSEL]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_CAROUSEL]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_CAROUSEL]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_CAROUSEL], true);
	modelStack.PopMatrix();

	//modelStack.PushMatrix();
	//modelStack.Translate(light->position.x, 4.f, light->position.z);
	//modelStack.Scale(1.f, 10.f, 1.f);
	//modelStack.Rotate(0.f, 0.f, 1.f, 0.f);
	//RenderMesh(meshList[GEO_BALLOONSTRING], false);
	//modelStack.PopMatrix();

	//modelStack.PushMatrix();
	//modelStack.Translate(rimPosition.x, rimPosition.y - 0.5f, rimPosition.z);
	//modelStack.Scale(0.2f, 0.2, 0.2f);
	//RenderMesh(meshList[GEO_SPHERE], false);
	//modelStack.PopMatrix();

	if (showstart == true) {
		if (inStartArea == true) {
			modelStack.PushMatrix();
			RenderTextOnScreen(meshList[GEO_TEXT], "Press R to start! (Shoot when you are ready)", glm::vec3(1, 1, 1), 20, 140, 200);
			modelStack.PopMatrix();
		}
	}

	/*if (OverlapCircle2Circle(camera.position, pickupDistance, ball.pos, ballRadius)) {
		modelStack.PushMatrix();
		modelStack.Scale(1.f, 1.f, 1.f);
		RenderTextOnScreen(meshList[GEO_TEXT], "Press E to pick up", glm::vec3(0, 1, 1), 20, 392, 200);
		modelStack.PopMatrix();
	}*/

	if (restartgame) {
		std::string point("Points: " + std::to_string(pointcounter));
		RenderTextOnScreen(meshList[GEO_TEXT], point.substr(0, 11), glm::vec3(0, 1, 0), 40, 0, 0);

		std::string timer("Timer: " + std::to_string(totalElapsedTime));
		RenderTextOnScreen(meshList[GEO_TEXT], timer.substr(0, 10), glm::vec3(0, 1, 0), 40, 0, 530);
	}

	std::string temp("FPS:" + std::to_string(fps));
	RenderTextOnScreen(meshList[GEO_TEXT], temp.substr(0, 9), glm::vec3(0, 1, 0), 40, 0, 560);

	if (showScore == true) {
		std::string point("Final Score: " + std::to_string(pointcounter));
		RenderTextOnScreen(meshList[GEO_TEXT], point.substr(0, 15), glm::vec3(0, 1, 0), 40, 238, 282);
		RenderTextOnScreen(meshList[GEO_TEXT], "(Press C to close and reopen score)", glm::vec3(1, 1, 1), 20, 238, 262);
		showCrosshair = false;
	}

	if (showCrosshair == true) {
		//tuff crosshair
		modelStack.PushMatrix();
		modelStack.Scale(1.f, 1.f, 1.f);
		RenderTextOnScreen(meshList[GEO_TEXT], "+", glm::vec3(0, 1, 1), 40, 392, 282);
		modelStack.PopMatrix();
	}
}

void Scene03::RenderMesh(Mesh* mesh, bool enableLight)
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


void Scene03::Exit()
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

void Scene03::HandleKeyPress(double dt)
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
	glm::vec3 forward = glm::normalize(camera.target - camera.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));

	// Use IsKeyDown for continuous movement while holding the key
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W))
	{
		// Move forward

		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
			float sprintmovement = (1.5 * moveSpeed) * static_cast<float>(dt);
			camera.position += forward * sprintmovement;
			camera.target += forward * sprintmovement;
		}
		else {
			float movement = moveSpeed * static_cast<float>(dt);
			camera.position += forward * movement;
			camera.target += forward * movement;
		}
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
	{
		// Move backward

		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
			float sprintmovement = (1.5 * moveSpeed) * static_cast<float>(dt);
			camera.position -= forward * sprintmovement;
			camera.target -= forward * sprintmovement;
		}
		else {
			float movement = moveSpeed * static_cast<float>(dt);
			camera.position -= forward * movement;
			camera.target -= forward * movement;
		}
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A))
	{
		// Move left (strafe)

		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
			float sprintmovement = (1.5 * moveSpeed) * static_cast<float>(dt);
			camera.position -= right * sprintmovement;
			camera.target -= right * sprintmovement;
		}
		else {
			float movement = moveSpeed * static_cast<float>(dt);
			camera.position -= right * movement;
			camera.target -= right * movement;
		}
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D))
	{
		// Move right (strafe)

		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
			float sprintmovement = (1.5 * moveSpeed) * static_cast<float>(dt);
			camera.position += right * sprintmovement;
			camera.target += right * sprintmovement;
		}
		else {
			float movement = moveSpeed * static_cast<float>(dt);
			camera.position += right * movement;
			camera.target += right * movement;
		}
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_SPACE)) {
		float fly = moveSpeed * static_cast<float>(dt);
		camera.position.y += fly;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
		float fly = moveSpeed * static_cast<float>(dt);
		camera.position.y -= fly;
	}

	/*if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
		camera.position.y = 0.5f;
	}
	else if (KeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_LEFT_CONTROL)) {
		camera.position.y = 2.f;
	}*/
}
