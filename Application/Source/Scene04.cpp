#include "Scene04.h"
#include "Mesh.h"
#include "GL\glew.h"
//kyler
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
#include <cmath>
#include <algorithm>

// repo cloning text test

Scene04::Scene04()
{
}

Scene04::~Scene04()
{
}

void Scene04::Init()
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

	m_parameters[U_LIGHT1_TYPE] = glGetUniformLocation(m_programID, "lights[1].type");
	m_parameters[U_LIGHT1_POSITION] = glGetUniformLocation(m_programID, "lights[1].position_cameraspace");
	m_parameters[U_LIGHT1_COLOR] = glGetUniformLocation(m_programID, "lights[1].color");
	m_parameters[U_LIGHT1_POWER] = glGetUniformLocation(m_programID, "lights[1].power");
	m_parameters[U_LIGHT1_KC] = glGetUniformLocation(m_programID, "lights[1].kC");
	m_parameters[U_LIGHT1_KL] = glGetUniformLocation(m_programID, "lights[1].kL");
	m_parameters[U_LIGHT1_KQ] = glGetUniformLocation(m_programID, "lights[1].kQ");
	m_parameters[U_LIGHT1_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[1].spotDirection");
	m_parameters[U_LIGHT1_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[1].cosCutoff");
	m_parameters[U_LIGHT1_COSINNER] = glGetUniformLocation(m_programID, "lights[1].cosInner");
	m_parameters[U_LIGHT1_EXPONENT] = glGetUniformLocation(m_programID, "lights[1].exponent");

	m_parameters[U_COLOR_TEXTURE_ENABLED] = glGetUniformLocation(m_programID, "colorTextureEnabled");
	m_parameters[U_COLOR_TEXTURE] = glGetUniformLocation(m_programID, "colorTexture");
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
	m_parameters[U_NUMLIGHTS] = glGetUniformLocation(m_programID, "numLights");

	Mesh::SetMaterialLoc(m_parameters[U_MATERIAL_AMBIENT], m_parameters[U_MATERIAL_DIFFUSE], m_parameters[U_MATERIAL_SPECULAR], m_parameters[U_MATERIAL_SHININESS]);

	// Initialise camera properties
	//camera.Init(45.f, 45.f, 10.f);

	camera.Init(
		glm::vec3(4.0f, 3.0f, 0.0f), // position: Y = 1.0f
		glm::vec3(0.0f, 1.0f, 0.0f), // target:  Y = 1.0f (same height -> no pitch)
		glm::vec3(0.0f, 1.0f, 0.0f)  // world up
	);

	// Init VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = nullptr;
	}

	//debug
	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
	

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

	
	//shapes
	meshList[GEO_SPHERE] = MeshBuilder::GenerateSphere("Sun", glm::vec3(1.f, 1.f, 1.f), 1.f, 16, 16);
	meshList[GEO_QUAD] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_CYLINDER] = MeshBuilder::GenerateCylinder("Cylinder", glm::vec3(1.f, 1.f, 1.f), 36, 1.f, 2.f);
	meshList[GEO_CUBE] = MeshBuilder::GenerateCube("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);

	//ground
	meshList[GEO_GRASS] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_GRASS]->textureID = LoadTGA("Images//coast_sand_rocks_02 copy.tga");
	//models
	
	meshList[GEO_DEER] = MeshBuilder::GenerateOBJMTL("demon","Models//model_containment//obj//musk_deer.obj","Models//model_containment//mtl//musk_deer.mtl");
	meshList[GEO_DEER]->textureID = LoadTGA("Images//model_containment//textures//musk_deer.tga");

	meshList[GEO_COW] = MeshBuilder::GenerateOBJMTL("lowkeychillguy", "Models//model_containment//obj//cow.obj", "Models//model_containment//mtl//cow.mtl");
	meshList[GEO_COW]->textureID = LoadTGA("Images//model_containment//textures//cow.tga");

	meshList[GEO_SHEEP] = MeshBuilder::GenerateOBJMTL("demon", "Models//model_containment//obj//sheep.obj", "Models//model_containment//mtl//sheep.mtl");
	meshList[GEO_SHEEP]->textureID = LoadTGA("Images//model_containment//textures//sheep.tga");

	meshList[GEO_BUCKET] = MeshBuilder::GenerateOBJMTL("dog", "Models//model_containment//obj//rv_bucket.obj", "Models//model_containment//mtl//rv_bucket.mtl");
	meshList[GEO_BUCKET]->textureID = LoadTGA("Images//model_containment//textures//goal_bucket.tga");

	meshList[GEO_TENT] = MeshBuilder::GenerateOBJMTL("carnival tent", "Models//tent//carnivaltent.obj", "Models//tent//carnivaltent.mtl");

	meshList[GEO_TREE] = MeshBuilder::GenerateOBJMTL("tree", "Models//birchtree//birchtree.obj", "Models//birchtree/birchtree.mtl");
	meshList[GEO_TREE]->textureID = LoadTGA("Images//birchtree/birchtree_baseColor.tga");

	// 16 x 16 is the number of columns and rows for the text
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//Georgia.tga");


	meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_GUI]->textureID = LoadTGA("Images//blackblack.tga");

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);


	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(0.f, -1.0, 0.f);
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

	light[1].position = glm::vec3(0, 0, 0);
	light[1].color = glm::vec3(1, 1, 0.5);
	light[1].type = Light::LIGHT_POINT;
	light[1].power = 1;
	light[1].kC = 1.f;
	light[1].kL = 0.01f;
	light[1].kQ = 0.001f;
	light[1].cosCutoff = 4.f;
	light[1].cosInner = 30.f;
	light[1].exponent = 3.f;
	light[1].spotDirection = glm::vec3(0.f, 1.f, 0.f);

	glUniform3fv(m_parameters[U_LIGHT1_COLOR], 1, &light[1].color.r);
	glUniform1i(m_parameters[U_LIGHT1_TYPE], light[1].type);
	glUniform1f(m_parameters[U_LIGHT1_POWER], light[1].power);
	glUniform1f(m_parameters[U_LIGHT1_KC], light[1].kC);
	glUniform1f(m_parameters[U_LIGHT1_KL], light[1].kL);
	glUniform1f(m_parameters[U_LIGHT1_KQ], light[1].kQ);
	glUniform1f(m_parameters[U_LIGHT1_COSCUTOFF], cosf(glm::radians<float>(light[1].cosCutoff)));
	glUniform1f(m_parameters[U_LIGHT1_COSINNER], cosf(glm::radians<float>(light[1].cosInner)));
	glUniform1f(m_parameters[U_LIGHT1_EXPONENT], light[1].exponent);

	enableLight = true;

	m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
	m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//ball innit
	for (int i = 0; i < ball_num; i++) {
		bounce_balls[i].ball.mass = 10;
		bounce_balls[i].ball.bounciness = 1;
		bounce_balls[i].ball.pos.y = 10;
		bounce_balls[i].ball.pos.x = 2*i;
	}
	//baucket innit
	for (int i = 0; i < ring_num; i++) {
		rings[i].bucket.bounciness = 1;
	}

	rings[0].bucket.pos.z = rings[0].radius*2;
	rings[1].bucket.pos.x = rings[0].radius * 2;
	rings[1].bucket.pos.z = -rings[0].radius * 2;
	rings[2].bucket.pos.x = -rings[0].radius * 2;
	rings[2].bucket.pos.z = -rings[0].radius * 2;

	green_cow.pos = glm::vec3(-25.f, 0.f, 0.f);

	player.mass = 0;
	player.bounciness = 1;
	player.pos.y = 10;
	player.pos.x = 10;

	floor.mass = 0;
	floor.bounciness = 1;
	floor.pos.y = 0;
	floor.pos.x = 0;

	glm::vec3 view = glm::normalize(camera.target - camera.position);

	camera.position.x += -135;

	camera.target = camera.position + view;

}



void Scene04::Update(double dt)
{
	player.pos = camera.position;
	
	//physics
	balls_update(dt);
	//handle inputs
	HandleMouseInput();
	HandleKeyPress(dt);

	// Prevent camera from going below ground after camera updates
	float ch;
	if (playing == true) {
		ch = 15.0f;
		if (change_height == true) {
			camera.position.y = ch;
			camera.target.y = ch;
			change_height = false;
		}
		
	}
	else{
		ch = 3.0f;
		if (change_height == true) {
			camera.position.y = ch;
			camera.target.y = ch;
			change_height = false;
		}
		
	}
	if (camera.position.y < ch) {
		camera.position.y = ch;
		if (camera.target.y < ch)
			camera.target.y = ch;
		glm::vec3 view = glm::normalize(camera.target - camera.position);

		camera.target = camera.position + view;

		camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		//camera.Init(camera.position, camera.target, camera.up);
	}

	if (OverlapCircle2(player.pos, 5.f, green_cow.pos, 5.f)){
		playing = true;
		change_height = true;
	}


	int score = 0;
	for (int i = 0; i < ball_select; i++) {
		if (bounce_balls[i].in == true) {
			score++;
		}
	}
	std::cout << score << std::endl;

	if (ball_select >= 10) {
		playing = false;
		std::string result = "you have scored an amazing "+(std::to_string(score)+" balls");
		send_message(result);
		change_height  = true;
		for (int i = 0; i < ball_num; i++) {
			bounce_balls[i].thrown = false;
		}
		ball_select = 0;
		camera.position.x = -50;
	}

	score = 0;
	camera.Update(dt);
}

void Scene04::balls_update(double dt) {
	for (int i = 0; i < ball_num; i++) {
		
		//collisions
		// ball against ball
		for (int j = i + 1; j < ball_num; j++) {
			if (OverlapCircle2Circle(bounce_balls[i].ball, bounce_balls[i].radius, bounce_balls[j].ball, bounce_balls[j].radius,cd)) {
				ResolveCollisionBall(cd);
			}
		}
		//ball against floor
		if (OverlapCircle2AABB(bounce_balls[i].ball, bounce_balls[i].radius, floor, glm::vec3 (floor_space, floor_height, floor_space),cd)) {
			walls_resolve(cd);
		}

		//ball agaisnt buckets
		for (int l = 0; l < ring_num; l++) {
			rings[l].wall[0].pos = glm::vec3(
				rings[l].bucket.pos.x,
				rings[l].bucket.pos.y + rings[l].height,
				rings[l].bucket.pos.z + rings[l].radius);

			rings[l].wall[1].pos = glm::vec3(
				rings[l].bucket.pos.x + rings[l].radius,
				rings[l].bucket.pos.y + rings[l].height,
				rings[l].bucket.pos.z);

			rings[l].wall[2].pos = glm::vec3(
				rings[l].bucket.pos.x,
				rings[l].bucket.pos.y + rings[l].height,
				rings[l].bucket.pos.z - rings[l].radius);

			rings[l].wall[3].pos = glm::vec3(
				rings[l].bucket.pos.x - rings[l].radius,
				rings[l].bucket.pos.y + rings[l].height,
				rings[l].bucket.pos.z);

			// Define wall half sizes correctly
			glm::vec3 halfSizeFrontBack(
				rings[l].radius * 2,
				rings[l].height,
				rings[l].radius * 2);

			glm::vec3 halfSizeLeftRight(
				rings[l].radius * 2,
				rings[l].height,
				rings[l].radius * 2);

			// Check 4 walls
			for (int j=0; j < 4; j++) {
				CollisionData cd;
				if (OverlapSphere2OBB(bounce_balls[i].ball,rings[l].wall[j],cd))
				{
					ResolveCollision(cd);
				}
			}
		}
		bool insideAny = false;

		for (int l = 0; l < ring_num; l++) {

			glm::vec3 halfSize(
				rings[l].radius*2,
				rings[l].height,
				rings[l].radius*2
			);

			if (OverlapSphere2OBB(
				bounce_balls[i].ball,
				rings[l].bucket,
				cd))
			{
				// Make sure ball is below rim
				if (bounce_balls[i].ball.pos.y <
					rings[l].bucket.pos.y + rings[l].height)
				{
					insideAny = true;
					break;
				}
			}
		}

		bounce_balls[i].in = insideAny;
		
	}
	
	static bool isLeftUp = false;
	static bool isRightUp = false;

	if (!isLeftUp && MouseController::GetInstance()->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {

		isLeftUp = true;
		if (playing == true) {
			if (ball_select < 10) {
				std::cout << bounce_balls[ball_select].ball.mass << std::endl;
				bounce_balls[ball_select].ball.vel = glm::vec3(0.f);
				//camera.target
				glm::vec3 direction = camera.target - camera.position;
				bounce_balls[ball_select].ball.pos = camera.position;
				bounce_balls[ball_select].ball.AddImpulse(direction * ball_power);
				bounce_balls[ball_select].ball.rotation = direction;
				std::cout << bounce_balls[ball_select].ball.rotation.x << std::endl;
				bounce_balls[ball_select].thrown = true;
				ball_select++;
			}
		}
	}
	else if (isLeftUp && MouseController::GetInstance()->IsButtonUp(GLFW_MOUSE_BUTTON_LEFT))
	{
		isLeftUp = false;
	}

	for (int i = 0; i < ball_num; i++) {
		//gravity 
		bounce_balls[i].ball.AddForce(glm::vec3(0, gravity*(bounce_balls[i].ball.mass), 0));
		
		//resolve collision
		bounce_balls[i].ball.UpdatePhysics(dt);
	}
}

void Scene04::Render()
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
	for (int i = 0; i < NUM_LIGHTS; i++) {
		if (light[i].type == Light::LIGHT_DIRECTIONAL)
		{
			glm::vec3 lightDir(light[i].position.x, light[i].position.y, light[i].position.z);
			glm::vec3 lightDirection_cameraspace = viewStack.Top() * glm::vec4(lightDir, 0);
			glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, glm::value_ptr(lightDirection_cameraspace));
		}
		else if (light[i].type == Light::LIGHT_SPOT)
		{
			glm::vec3 lightPosition_cameraspace = viewStack.Top() * glm::vec4(light[i].position, 1);
			glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, glm::value_ptr(lightPosition_cameraspace));
			glm::vec3 spotDirection_cameraspace = viewStack.Top() * glm::vec4(light[i].spotDirection, 0);
			glUniform3fv(m_parameters[U_LIGHT0_SPOTDIRECTION], 1, glm::value_ptr(spotDirection_cameraspace));
		}
		else {
			// Calculate the light position in camera space
			glm::vec3 lightPosition_cameraspace = viewStack.Top() * glm::vec4(light[i].position, 1);
			glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, glm::value_ptr(lightPosition_cameraspace));
		}
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
	RenderSkybox(skyboxscale,player.pos);

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
				meshList[GEO_GRASS]->material.kAmbient = glm::vec3(1.f, 1.f, 0.f);
				meshList[GEO_GRASS]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.1f);
				meshList[GEO_GRASS]->material.kSpecular = glm::vec3(0.1f, 0.1f, 0.1f);
				meshList[GEO_GRASS]->material.kShininess = 1.0f;
				RenderMesh(meshList[GEO_GRASS], true);
				modelStack.PopMatrix();
			}
		}
		// keep the ambient material tweak from original code
	}
	modelStack.PopMatrix();
	meshList[GEO_TREE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_TREE]->material.kDiffuse = glm::vec3(0.8, 0, 0);
	meshList[GEO_TREE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_TREE]->material.kShininess = 1.0f;

	meshList[GEO_TENT]->material.kAmbient = glm::vec3(0.1, 0.1, 0.1);
	meshList[GEO_TENT]->material.kDiffuse = glm::vec3(0.8, 0, 0);
	meshList[GEO_TENT]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_TENT]->material.kShininess = 5.0f;

	balls_render();
	walls_render();
	buckets_render();
	models_render();
	trees_render();
	tree_render();
	cows_render();

	RenderTextOnScreen(meshList[GEO_TEXT],anouncement,glm::vec3(1.f,0.f,0.f),25, 10, 550);
}

void Scene04::cows_render() {
	modelStack.PushMatrix();
	modelStack.Translate(-25.f, 0.f, 0.f);
	modelStack.Scale(1.f, 1.f, 1.f);
	modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
	// keep original rotations so the tile faces the same way as before
	meshList[GEO_COW]->material.kAmbient = glm::vec3(0.0f, 1.f, 0.0f);
	meshList[GEO_COW]->material.kDiffuse = glm::vec3(1.0f, 1.0f, 0.0f);
	meshList[GEO_COW]->material.kSpecular = glm::vec3(0.3f, 0.3f, 0.3f);
	meshList[GEO_COW]->material.kShininess = 1.f;
	RenderMesh(meshList[GEO_COW], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(-75.f, 0.f, -25.f);
	modelStack.Scale(1.f, 1.f, 1.f);
	//modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
	// keep original rotations so the tile faces the same way as before
	meshList[GEO_COW]->material.kAmbient = glm::vec3(1.f, 0.f, 0.0f);
	meshList[GEO_COW]->material.kDiffuse = glm::vec3(1.0f, 1.0f, 0.0f);
	meshList[GEO_COW]->material.kSpecular = glm::vec3(0.3f, 0.3f, 0.3f);
	meshList[GEO_COW]->material.kShininess = 1.f;
	RenderMesh(meshList[GEO_COW], true);
	modelStack.PopMatrix();
};

void Scene04::models_render() {
	modelStack.PushMatrix();
	modelStack.Translate(-100.f, 0.f, 0.f);
	modelStack.Scale(1.f, 1.f, 1.f);
	// keep original rotations so the tile faces the same way as before
	RenderMesh(meshList[GEO_TENT], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(-130.f, 0.f, -65.f);
	modelStack.Scale(2.1f, 2.5f, 1.f);
	// keep original rotations so the tile faces the same way as before
	meshList[GEO_CUBE]->material.kAmbient = glm::vec3(0.f, 0.f, 0.f);
	meshList[GEO_CUBE]->material.kDiffuse = glm::vec3(0, 0, 0);
	meshList[GEO_CUBE]->material.kSpecular = glm::vec3(0.0f, 0.0f, 0.0f);
	meshList[GEO_CUBE]->material.kShininess = 1.0f;
	RenderMesh(meshList[GEO_CUBE], true);
	modelStack.PopMatrix();
}
void Scene04::trees_render() {
	int ii;

	for (int i = 0; i < 25; i++) {
		ii = (i * 10);

		modelStack.PushMatrix();
		modelStack.Translate(200.f, 0.f, ii);
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		// keep original rotations so the tile faces the same way as before
		RenderMesh(meshList[GEO_TREE], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(200.f, 0.f, -ii);
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		// keep original rotations so the tile faces the same way as before
		RenderMesh(meshList[GEO_TREE], true);

		modelStack.PopMatrix();
		modelStack.PushMatrix();
		modelStack.Translate(-200.f, 0.f, ii);
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		// keep original rotations so the tile faces the same way as before
		RenderMesh(meshList[GEO_TREE], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(-200.f, 0.f, -ii);
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		// keep original rotations so the tile faces the same way as before
		RenderMesh(meshList[GEO_TREE], true);
		modelStack.PopMatrix();
	}
};
void Scene04::tree_render() {
	
	int ii;

	for (int i = 0; i < 25; i++) {
		ii = (i * 10);

		modelStack.PushMatrix();
		modelStack.Translate(ii, 0.f, 200.f);
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		// keep original rotations so the tile faces the same way as before
		RenderMesh(meshList[GEO_TREE], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(-ii, 0.f,200.f );
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		// keep original rotations so the tile faces the same way as before
		RenderMesh(meshList[GEO_TREE], true);

		modelStack.PopMatrix();
		modelStack.PushMatrix();
		modelStack.Translate(-ii, 0.f,- 200.f);
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		// keep original rotations so the tile faces the same way as before
		RenderMesh(meshList[GEO_TREE], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(ii, 0.f, -200.f);
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		// keep original rotations so the tile faces the same way as before
		RenderMesh(meshList[GEO_TREE], true);
		modelStack.PopMatrix();
	}
}

void Scene04::balls_render() {
	for (int i = 0; i < ball_num; i++) {
		if (bounce_balls[i].thrown == true) {
			if (show_col == true) {
				modelStack.PushMatrix();
				modelStack.Translate(bounce_balls[i].ball.pos.x, bounce_balls[i].ball.pos.y, bounce_balls[i].ball.pos.z);
				modelStack.Scale((bounce_balls[i].radius), (bounce_balls[i].radius), (bounce_balls[i].radius));
				meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
				meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
				meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
				meshList[GEO_SPHERE]->material.kShininess = 5.0f;
				RenderMesh(meshList[GEO_SPHERE], true);
				modelStack.PopMatrix();
			}
			
			if (i< 1) {
				modelStack.PushMatrix();
				modelStack.Translate(bounce_balls[i].ball.pos.x, bounce_balls[i].ball.pos.y, bounce_balls[i].ball.pos.z);
				modelStack.Scale((bounce_balls[i].radius), (bounce_balls[i].radius), (bounce_balls[i].radius));
				meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
				meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
				meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
				meshList[GEO_SPHERE]->material.kShininess = 5.0f;
				RenderMesh(meshList[GEO_SPHERE], true);
				modelStack.PopMatrix();
			}
			else if (i<5) {
				modelStack.PushMatrix();
				modelStack.Translate(bounce_balls[i].ball.pos.x, bounce_balls[i].ball.pos.y - (.5f), bounce_balls[i].ball.pos.z);
				modelStack.Scale((bounce_balls[i].radius), (bounce_balls[i].radius), (bounce_balls[i].radius));
				meshList[GEO_SHEEP]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[GEO_SHEEP]->material.kDiffuse = glm::vec3(0.1f, 0.1f, 0.1f);
				meshList[GEO_SHEEP]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
				meshList[GEO_SHEEP]->material.kShininess = 1.0f;
				RenderMesh(meshList[GEO_SHEEP], true);
				modelStack.PopMatrix();
			}
			
			else {
				modelStack.PushMatrix();
				modelStack.Translate(bounce_balls[i].ball.pos.x, bounce_balls[i].ball.pos.y - (.5f), bounce_balls[i].ball.pos.z);
				modelStack.Scale((bounce_balls[i].radius), (bounce_balls[i].radius), (bounce_balls[i].radius));
				meshList[GEO_DEER]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[GEO_DEER]->material.kDiffuse = glm::vec3(0.1f, 0.1f, 0.1f);
				meshList[GEO_DEER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
				meshList[GEO_DEER]->material.kShininess = 1.0f;
				RenderMesh(meshList[GEO_DEER], true);
				modelStack.PopMatrix();
			}
		}
	}
}

void Scene04::walls_render() {
	modelStack.PushMatrix();
	meshList[GEO_CUBE]->material.kAmbient = glm::vec3(0.f, 0.1f, 0.f);
	meshList[GEO_CUBE]->material.kDiffuse = glm::vec3(0.f, 0.1f, 0.f);
	meshList[GEO_CUBE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_CUBE]->material.kShininess = 5.0f;
	modelStack.Translate(floor.pos.x, floor.pos.y, floor.pos.z);
	modelStack.Scale(floor_space / 10, floor_height / 10, floor_space / 10);
	modelStack.Rotate(0, 1.f, 1.f, 1.f);
	RenderMesh(meshList[GEO_CUBE], true);
	modelStack.PopMatrix();
}

void Scene04::buckets_render() {
	for (int i = 0; i < ring_num; i++) {
		if (show_col == true) {
			meshList[GEO_CUBE]->material.kAmbient = glm::vec3(0.f, 1.f, 1.f);
			meshList[GEO_CUBE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
			meshList[GEO_CUBE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
			meshList[GEO_CUBE]->material.kShininess = 5.0f;
			modelStack.Translate(rings[i].bucket.pos.x, rings[i].bucket.pos.y / 2, rings[i].bucket.pos.z);

			modelStack.Scale(rings[i].radius, rings[i].height, rings[i].radius);
			modelStack.Rotate(0, 1.f, 1.f, 1.f);
			RenderMesh(meshList[GEO_CUBE], true);
			modelStack.PopMatrix();
		}

		modelStack.PushMatrix();
		modelStack.Translate(rings[i].bucket.pos.x, rings[i].bucket.pos.y, rings[i].bucket.pos.z);
		modelStack.Scale((rings[i].radius), (rings[i].height) / 2.5, (rings[i].radius));
		meshList[GEO_BUCKET]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
		meshList[GEO_BUCKET]->material.kDiffuse = glm::vec3(0.1f, 0.1f, 0.1f);
		meshList[GEO_BUCKET]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_BUCKET]->material.kShininess = 1.0f;
		RenderMesh(meshList[GEO_BUCKET], true);
		modelStack.PopMatrix();
	}
}

void Scene04::RenderSkybox(float scale, glm::vec3 camerapos)
{
	modelStack.PushMatrix();
	// Offset in Z direction by -50 units
	modelStack.Translate(camerapos.x, camerapos.y, camerapos.z - (50 * scale));
	modelStack.Scale(scale, scale, scale);
	meshList[GEO_FRONT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_FRONT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	// Offset in Z direction by -50 units
	modelStack.Translate(camerapos.x, camerapos.y, camerapos.z + (50 * scale));
	modelStack.Scale(scale, scale, scale);
	modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
	meshList[GEO_BACK]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_BACK], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camerapos.x - (50 * scale), camerapos.y, camerapos.z);
	modelStack.Scale(scale, scale, scale);
	modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
	meshList[GEO_LEFT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_LEFT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camerapos.x + (50 * scale), camerapos.y, camerapos.z);
	modelStack.Scale(scale, scale, scale);
	modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
	meshList[GEO_RIGHT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_RIGHT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camerapos.x, camerapos.y + (50 * scale), camerapos.z);
	modelStack.Scale(scale, scale, scale);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
	meshList[GEO_TOP]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_TOP], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camerapos.x, camerapos.y - (50 * scale), camerapos.z);
	modelStack.Scale(scale, scale, scale);
	modelStack.Rotate(90.f, -1.f, 0.f, 0.f);
	meshList[GEO_BOTTOM]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_BOTTOM], false);
	modelStack.PopMatrix();
}

void Scene04::RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey)
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

void Scene04::RenderText(Mesh* mesh, std::string text, glm::vec3 color)
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


void Scene04::RenderTextOnScreen(Mesh* mesh, std::string
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



void Scene04::RenderMesh(Mesh* mesh, bool enableLight)
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


void Scene04::Exit()
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

void Scene04::HandleKeyPress(double dt){
	// Calculate forward and right vectors based on camera orientation
	glm::vec3 forward = glm::normalize(camera.target - camera.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));

	// Use IsKeyDown for continuous movement while holding the key
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W))
	{
		// Move forward
		glm::vec3 view = glm::normalize(camera.target - camera.position);

		camera.position.x += view.x * 0.1;
		camera.position.z += view.z * 0.1;

		camera.target = camera.position + view;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
	{
		//Move back
		glm::vec3 view = glm::normalize(camera.target - camera.position);

		camera.position.x -= view.x * 0.1;
		camera.position.z -= view.z * 0.1;

		camera.target = camera.position + view;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A))
	{
		// Move left (strafe)
		glm::vec3 view = glm::normalize(camera.target - camera.position);
		glm::vec3 right = glm::normalize(glm::cross(view, camera.up));

		camera.position -= right * glm::vec3(0.1);// *speed; 
		camera.target -= right * glm::vec3(0.1);// *speed;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D))
	{
			// Move right (strafe)
		glm::vec3 view = glm::normalize(camera.target - camera.position);
		glm::vec3 right = glm::normalize(glm::cross(view, camera.up));

		camera.position += right * glm::vec3(0.1);// *speed; 
		camera.target += right * glm::vec3(0.1);// *speed;
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_J))
	{
		go_back();
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_E))
	{
		ball_power++;
		if (ball_power >= power_max) {
			ball_power = power_max;
		}
		std::string result = "current throwing strength: " + (std::to_string(ball_power));
		send_message(result);
	}
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_Q))
	{
		ball_power--;
		if (ball_power <= power_min) {
			ball_power = power_min;
		}
		std::string result = "current throwing strength: " + (std::to_string(ball_power));
		send_message(result);
	}
}

void Scene04::HandleMouseInput() {

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

	//camera.target = camera.position + glm::normalize(dir);

	// Re-init so FPCamera::Refresh() recalculates 'up' and other derived vectors
	glm::vec3 view = glm::normalize(camera.target - camera.position);

	camera.target = camera.position + view;

	camera.up = glm::vec3(0.0f, 1.0f, 0.0f);

	//camera.Init(camera.position, camera.target, glm::vec3(0.0f, 1.0f, 0.0f));
}

bool Scene04::OverlapCircle2CYLINDER(const glm::vec3& pos1, float r1, const glm::vec3& pos2, float width, float height){
	glm::vec3 d = pos1 - pos2;
	float lengthSq = glm::dot(d, d);
	float rsum = r1 + width;
	return lengthSq <= rsum * rsum;
}

void Scene04::ResolveCollisionBall(CollisionData cd) {

	PhysicsObject&  o1 = *cd.pObj1;
	PhysicsObject& o2 = *cd.pObj2;
	
	glm::vec3 oc = (cd.collisionNormal * cd.penetration);

	oc.x = oc.x / 2;
	oc.y = oc.y / 2;
	oc.z = oc.z / 2;

    if (o2.mass > 0.f) {
		o2.pos += oc;
		//o2.vel = (o1.vel * (o2.bounciness));
		o2.AddImpulse((cd.collisionNormal * (o2.bounciness))*o2.mass);
		o2.angularVel = o2.vel;
	}
	if (o1.mass > 0.f) {
		o1.pos -= oc;
		//o1.vel = -(o2.vel * (o1.bounciness));
		o1.AddImpulse((-cd.collisionNormal * (o1.bounciness))* o1.mass);
		o1.angularVel = o1.vel;
	}
}

bool Scene04::OverlapCircle2(const glm::vec3& pos1, float r1, const glm::vec3& pos2, float r2)
{
	float x = pos1.x - pos2.x;
	//float y = pos1.y - pos2.y;
	float z = pos1.z - pos2.z;
	float r = r1 + r2;
	return (x * x + z * z) <= (r * r);
}

void Scene04::walls_resolve(CollisionData cd) {

	PhysicsObject& o1 = *cd.pObj1;
	PhysicsObject& o2 = *cd.pObj2;

	glm::vec3 n = glm::normalize(cd.collisionNormal);

	float invMass1 = (o1.mass == 0.f) ? 0.f : 1.f / o1.mass;
	float invMass2 = (o2.mass == 0.f) ? 0.f : 1.f / o2.mass;
	float totalInvMass = invMass1 + invMass2;

	if (totalInvMass == 0.f)
		return;

	// -------------------------------
	// 1) POSITIONAL CORRECTION
	// -------------------------------
	float percent = 1.f; // correction percentage
	glm::vec3 correction = (cd.penetration / totalInvMass) * percent * n;

	o1.pos += correction * invMass1;
	o2.pos -= correction * invMass2;

	// -------------------------------
	// 2) NORMAL IMPULSE (BOUNCE)
	// -------------------------------
	glm::vec3 relativeVel = o1.vel - o2.vel;
	float velAlongNormal = glm::dot(relativeVel, n);

	if (velAlongNormal > 0.f)
		return; // separating already

	float restitution = std::fmin(o1.bounciness, o2.bounciness);

	float j = -(1.f + restitution) * velAlongNormal;
	j /= totalInvMass;

	glm::vec3 impulse = j * n;

	o1.vel += impulse * invMass1;
	o2.vel -= impulse * invMass2;

	// -------------------------------
	// 3) FRICTION
	// -------------------------------
	relativeVel = o1.vel - o2.vel;

	glm::vec3 tangent =
		relativeVel - glm::dot(relativeVel, n) * n;

	if (glm::length(tangent) > 0.0001f)
		tangent = glm::normalize(tangent);
	else
		return;

	float jt = -glm::dot(relativeVel, tangent);
	jt /= totalInvMass;

	float mu = 0.3f; // friction coefficient

	// Coulomb friction clamp
	if (fabs(jt) > j * mu)
		jt = j * mu * (jt < 0.f ? -1.f : 1.f);

	glm::vec3 frictionImpulse = jt * tangent;

	o1.vel += frictionImpulse * invMass1;
	o2.vel -= frictionImpulse * invMass2;

	// -------------------------------
	// 4) Small velocity clamp
	// -------------------------------
	if (glm::length(o1.vel) < 0.01f)
		o1.vel = glm::vec3(0.f);
}

void Scene04::go_back() {
	send_message("goinhg back to clowntopia");
	scene01request = true;
}

void Scene04::send_message(std::string slring) {
	anouncement = slring;
}