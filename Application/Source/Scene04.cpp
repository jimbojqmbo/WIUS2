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
	meshList[GEO_DEER]->textureID = LoadTGA("Models//model_containment//textures//musk_deer.tga");

	meshList[GEO_COW] = MeshBuilder::GenerateOBJMTL("lowkeychillguy", "Models//model_containment//obj//cow.obj", "Models//model_containment//mtl//cow.mtl");
	meshList[GEO_COW]->textureID = LoadTGA("Models//model_containment//textures//cow.tga");

	meshList[GEO_SHEEP] = MeshBuilder::GenerateOBJMTL("demon", "Models//model_containment//obj//13574_Marco_Polo_Sheep_v1_L3.obj", "Models//model_containment//mtl//13574_Marco_Polo_Sheep_v1_L3.mtl");
	meshList[GEO_SHEEP]->textureID = LoadTGA("Models//model_containment//textures//13574_Marco_Polo_Diffuse.tga");

	meshList[GEO_BUCKET] = MeshBuilder::GenerateOBJMTL("lowkeychillguy", "Models//model_containment//obj//goal_bucket.obj", "Models//model_containment//mtl//goal_bucket.mtl");
	//meshList[GEO_BUCKET]->textureID = LoadTGA("Models//model_containment//textures//goal_bucket.tga");

	// 16 x 16 is the number of columns and rows for the text
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//Georgia.tga");

	meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_GUI]->textureID = LoadTGA("Images//blackblack.tga");

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);


	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(camera.position.x, camera.position.y, camera.position.z);
	light[0].color = glm::vec3(1, 1, 0.5);
	light[0].type = Light::LIGHT_POINT;
	light[0].power = 100;
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

	//ball innit
	for (int i = 0; i < ball_num; i++) {
		bounce_balls[i].ball.mass = 2;
		bounce_balls[i].ball.bounciness = 1;
		bounce_balls[i].ball.pos.y = 10;
		bounce_balls[i].ball.pos.x = 2*i;
	}
	player.mass = 0;
	player.bounciness = 1;
	player.pos.y = 10;
	player.pos.x = 10;

	floor.mass = 0;
	floor.bounciness = 1;
	floor.pos.y = 0;
	floor.pos.x = 0;
	

}



void Scene04::Update(double dt)
{
	player.pos = camera.position;
	
	//std::cout << player.pos.x<< " " <<player.pos.z << std::endl;
	//std::cout << ball[0].pos.x << " " << ball[0].pos.z << std::endl;
	//physics
	balls_update(dt);
	//handle inputs
	HandleMouseInput();
	HandleKeyPress(dt);

	// Prevent camera from going below ground after camera updates
	if (camera.position.y < 3.0f) {
		camera.position.y = 3.0f;
		if (camera.target.y < 3.0f)
			camera.target.y = 3.0f;
		camera.Init(camera.position, camera.target, camera.up);
	}

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
		//ball agaisnt player test
		if (OverlapCircle2Circle(bounce_balls[i].ball, bounce_balls[i].radius, player, bounce_balls[i].radius, cd)) {
			//std::cout << "ball_touching" << std::endl;
			//ResolveCollisionBall(cd);
		}
		//ball against floor
		if (OverlapCircle2AABB(bounce_balls[i].ball, bounce_balls[i].radius, floor, glm::vec3 (floor_space, floor_height, floor_space),cd)) {
			walls_resolve(cd);
			//std::cout << "ball collide with floor" << std::endl;
		}
		
	}
	
	static bool isLeftUp = false;
	static bool isRightUp = false;

	if (!isLeftUp && MouseController::GetInstance()->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {

		isLeftUp = true;
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
	else if (isLeftUp && MouseController::GetInstance()->IsButtonUp(GLFW_MOUSE_BUTTON_LEFT))
	{
		isLeftUp = false;
	}

	for (int i = 0; i < ball_num; i++) {
		//gravity 
		bounce_balls[i].ball.AddForce(glm::vec3(0, gravity, 0));
		
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
	RenderSkybox(skyboxscale,camera.position);

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

	balls_render();
	walls_render();
	buckets_render();
}

void Scene04::balls_render() {
	for (int i = 0; i < ball_num; i++) {
		if (show_col == true) {
			modelStack.PushMatrix();
			modelStack.Translate(bounce_balls[i].ball.pos.x, bounce_balls[i].ball.pos.y, bounce_balls[i].ball.pos.z);
			modelStack.Scale((bounce_balls[i].radius), (bounce_balls[i].radius), (bounce_balls[i].radius));
			modelStack.Rotate(bounce_balls[i].ball.rotation.x, 1.f, 0.f, 0.f);
			modelStack.Rotate(bounce_balls[i].ball.rotation.y, 0.f, 1.f, 0.f);
			modelStack.Rotate(bounce_balls[i].ball.rotation.z, 0.f, 0.f, 1.f);
			RenderMesh(meshList[GEO_SPHERE], true);
			modelStack.PopMatrix();
		}

		modelStack.PushMatrix();
		modelStack.Translate(bounce_balls[i].ball.pos.x, bounce_balls[i].ball.pos.y-(.5f), bounce_balls[i].ball.pos.z);
		modelStack.Scale((bounce_balls[i].radius), (bounce_balls[i].radius), (bounce_balls[i].radius));
		modelStack.Rotate(bounce_balls[i].ball.rotation.x, 1.f, 0.f, 0.f);
		modelStack.Rotate(bounce_balls[i].ball.rotation.y, 0.f, 1.f, 0.f);
		modelStack.Rotate(bounce_balls[i].ball.rotation.z, 0.f, 0.f, 1.f);
		RenderMesh(meshList[GEO_DEER], true);
		modelStack.PopMatrix();
	}
}

void Scene04::walls_render(){
	modelStack.PushMatrix();
	meshList[GEO_CUBE]->material.kAmbient = glm::vec3(0.f, 1.f, 1.f);
	meshList[GEO_CUBE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
	meshList[GEO_CUBE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GEO_CUBE]->material.kShininess = 5.0f;
	modelStack.Translate(floor.pos.x, floor.pos.y, floor.pos.z);
	modelStack.Scale(floor_space/10, floor_height/10, floor_space/10);
	modelStack.Rotate(0, 1.f, 1.f, 1.f);
	RenderMesh(meshList[GEO_CUBE], true);
	modelStack.PopMatrix();
}

void Scene04::buckets_render() {
	for (int i = 0; i < ring_num; i++) {
		modelStack.PushMatrix();
		modelStack.Translate(bounce_balls[i].ball.pos.x, bounce_balls[i].ball.pos.y - (.5f), bounce_balls[i].ball.pos.z);
		modelStack.Scale((bounce_balls[i].radius), (bounce_balls[i].radius), (bounce_balls[i].radius));
		modelStack.Rotate(bounce_balls[i].ball.rotation.x, 1.f, 0.f, 0.f);
		modelStack.Rotate(bounce_balls[i].ball.rotation.y, 0.f, 1.f, 0.f);
		modelStack.Rotate(bounce_balls[i].ball.rotation.z, 0.f, 0.f, 1.f);
		RenderMesh(meshList[GEO_BUCKET], true);
		modelStack.PopMatrix();
	}
}

void Scene04::RenderSkybox(float scale,glm::vec3 camerapos)
{
	modelStack.PushMatrix();
	// Offset in Z direction by -50 units
	modelStack.Translate(camerapos.x , camerapos.y, camerapos.z -(50*scale));
	modelStack.Scale(scale,scale,scale);
	meshList[GEO_FRONT]->material.kAmbient = glm::vec3(0.25f, 0.25f, 0.25f);
	RenderMesh(meshList[GEO_FRONT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	// Offset in Z direction by -50 units
	modelStack.Translate(camerapos.x , camerapos.y, camerapos.z +(50*scale));
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
	modelStack.Translate(camerapos.x , camerapos.y + (50 * scale), camerapos.z);
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

void Scene04::HandleKeyPress(double dt)
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
	/*
	if (KeyboardController::GetInstance()->IsKeyPressed(VK_SPACE))
	{
		// Change to black background
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_0))
	{
		//Toggle light on or off
	   /*enableLight = !enableLight;*/

	   //if (light[0].power <= 0.1f)
		   //light[0].power = 1.f;
	   //else
		   ///light[0].power = 0.1f;
	   //glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	//}
	/*
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
	*/
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
		// Move backward
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
	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_R))
	{
		for (int i = 0; i < ball_num; i++) {
			bounce_balls[i].thrown = false;
		}
		ball_select = 0;
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_C))
	{
		show_col = not(show_col);
	}
	
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
	// Clamp camera height to adjusted limits
	if (camera.position.y < 3.3f) {
		camera.position.y = 3.3f;
		if (camera.target.y < 3.3f)
			camera.target.y = 3.3f;
	}
	if (camera.position.y > 3.8f) {
		camera.position.y = 3.8f;
		if (camera.target.y > 3.8f)
			camera.target.y = 3.8f;
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

	camera.target = camera.position + glm::normalize(dir);

	// Re-init so FPCamera::Refresh() recalculates 'up' and other derived vectors
	camera.Init(camera.position, camera.target, glm::vec3(0.0f, 3.0f, 0.0f));
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
		o2.vel = (o1.vel * (o2.bounciness));
		o2.AddImpulse((cd.collisionNormal * (o2.bounciness))*o2.mass);
		o2.angularVel = o2.vel;
	}
	if (o1.mass > 0.f) {
		o1.pos -= oc;
		o1.vel = -(o2.vel * (o1.bounciness));
		o1.AddImpulse((-cd.collisionNormal * (o1.bounciness))* o1.mass);
		o1.angularVel = o1.vel;
	}
}

bool Scene04::OverlapCircle2(const glm::vec3& pos1, float r1, const glm::vec3& pos2, float r2)
{
	float x = pos1.x - pos2.x;
	float y = pos1.y - pos2.y;
	float z = pos1.z - pos2.z;
	float r = r1 + r2;
	return (x * x + y * y + z * z) <= (r * r);
}

void Scene04::walls_resolve(CollisionData cd) {

	PhysicsObject& o1 = *cd.pObj1;
	PhysicsObject& o2 = *cd.pObj2;

	glm::vec3 n = glm::normalize(cd.collisionNormal);

	float invMass1 = (o1.mass == 0.f) ? 0.f : 1.f / o1.mass;
	float invMass2 = (o2.mass == 0.f) ? 0.f : 1.f / o2.mass;
	float totalInvMass = invMass1 + invMass2;
	if (totalInvMass == 0.f) return;

	// --- Immediate positional correction ---
	o1.pos += n * cd.penetration; // fully move sphere out of wall

	// --- Compute relative velocity along normal ---
	float velAlongNormal = glm::dot(o1.vel - o2.vel, n);

	// --- Apply bounciness ---
	float restitution = std::fmin(o1.bounciness, o2.bounciness);

	if (velAlongNormal < 0.f) // only if moving into the wall
	{
		float j = -(1.f + restitution) * velAlongNormal / totalInvMass;
		glm::vec3 impulse = j * n;

		o1.vel += impulse * invMass1;
		o2.vel -= impulse * invMass2; // wall usually has invMass=0
		o1.angularVel = o1.vel;
		o2.angularVel = o2.vel;
	}

	// --- Friction along tangent ---
	glm::vec3 relativeVel = o1.vel - o2.vel;
	glm::vec3 tangent = relativeVel;// -glm::dot(relativeVel, n) * n;
	float lenT = glm::length(tangent);
	if (lenT > 0.001f)
	{
		tangent /= lenT;
		glm::vec3 frictionImpulse = -0.1f * tangent * glm::length(relativeVel); // simple friction
		o1.vel += frictionImpulse * invMass1;
		o2.vel -= frictionImpulse * invMass2;
		o1.angularVel = o1.vel;
		o2.angularVel = o2.vel;
	}

	// Clamp very small velocities
	if (glm::length(o1.vel) < 0.01f)o1.vel = glm::vec3(0.f);
}