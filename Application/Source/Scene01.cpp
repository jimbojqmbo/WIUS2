// ---------------------------------------------------------------
// jin ming's scene
// ---------------------------------------------------------------

#include "Scene01.h"
#include "Mesh.h"
#include "GL\glew.h"

#include "PhysicsObject.h"
#include <vector>
#include <algorithm> // for std::min/std::max
#include <chrono>

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
#include <iomanip>
#include <cmath> // for atan2, etc.

// repo cloning text test

Scene01::Scene01()
{
}

Scene01::~Scene01()
{
}

bool Scene01::IsInsideFence(const glm::vec3& p) const {
	// Use the fenceZones vector you populate in Init()
	for (const auto& box : fenceZones) {
		const glm::vec3& mn = box.first;
		const glm::vec3& mx = box.second;
		if (p.x >= mn.x && p.x <= mx.x &&
			p.y >= mn.y && p.y <= mx.y &&
			p.z >= mn.z && p.z <= mx.z)
			return true;
	}
	return false;
}

void Scene01::Init()
{
	BumperCarGameEntered = false;
	EnterBumperCarGamePrompt = false;
	ExitBumperCarGamePrompt = false;
	pausemenu = false;
	enterCar1Check = false;
	enterCar2Check = false;

	gravity = -30.0f;
	jumpSpeed = 10.0f;
	cameraVerticalVel1 = 0.0f;
	cameraVerticalVel2 = 0.0f;
	isJumping1 = false;
	isJumping2 = false;

	objectPos = glm::vec3(0.0f, 0.0f, 0.0f);
	moveSpeed = 5.0f; // units per second

	BasketballGameEntered = false;
	BallBouncerGameEntered = false;
	DuckShootingGameEntered = false;

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
		glm::vec3(14.2f, 3.3f, -37.5f),
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
	meshList[GEO_LEFT]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Left.tga");
	//meshList[GEO_LEFT]->textureID = LoadTGA("Images//whitesky//whiteskyleft.tga");
	//meshList[GEO_LEFT]->textureID = LoadTGA("Images//left.tga");

	meshList[GEO_RIGHT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_RIGHT]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Right.tga");
	//meshList[GEO_RIGHT]->textureID = LoadTGA("Images//whitesky//whiteskyright.tga");
	//meshList[GEO_RIGHT]->textureID = LoadTGA("Images//right.tga");

	meshList[GEO_BACK] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_BACK]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Back.tga");
	//meshList[GEO_BACK]->textureID = LoadTGA("Images//whitesky//whiteskyback.tga");
	//meshList[GEO_BACK]->textureID = LoadTGA("Images//back.tga");

	meshList[GEO_FRONT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_FRONT]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Front.tga");
	//meshList[GEO_FRONT]->textureID = LoadTGA("Images//whitesky//whiteskyfront.tga");
	//meshList[GEO_FRONT]->textureID = LoadTGA("Images//front.tga");

	meshList[GEO_TOP] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//saharatop.tga");
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//top.tga");
	//meshList[GEO_TOP]->textureID = LoadTGA("Images//whitesky//whiteskytop.tga");
	meshList[GEO_TOP]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Top.tga");

	meshList[GEO_BOTTOM] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 100.f);
	meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//AlvinSkybox//AlvinSkybox_Bottom.tga");
	//meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//whitesky//whiteskybottom.tga");
	//meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//bottom.tga");

	//meshList[GEO_QUAD]->textureID = LoadTGA("Images//NYP.tga");
	meshList[GEO_QUAD] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);

	meshList[GEO_CYLINDER] = MeshBuilder::GenerateCylinder("Cylinder", glm::vec3(1.f, 1.f, 1.f), 36, 1.f, 2.f);

	//meshList[GEO_GRASS] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	//meshList[GEO_GRASS]->textureID = LoadTGA("Images//coast_sand_rocks_02 copy.tga");

	meshList[SOLIDGREENGRASS] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[SOLIDGREENGRASS]->textureID = LoadTGA("Images//solidgreengrass.tga");

	//meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	//meshList[GEO_GUI]->textureID = LoadTGA("Images//color.tga");

	// 16 x 16 is the number of columns and rows for the text
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//cascadiamonosemibold.tga");

	//meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	//meshList[GEO_GUI]->textureID = LoadTGA("Images//blackblack.tga");

	//meshList[GEO_SHADOW] = MeshBuilder::GenerateOBJMTL("shadow", "Models//swamp_monster.obj", "Models//swamp_monster.mtl");
	//meshList[GEO_SHADOW]->textureID = LoadTGA("Images//FOREST_MONSTER_V1_mat_diffuse.tga");

	//meshList[GEO_ABANDONEDHOUSE] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//abandoned_house.obj", "Models//abandoned_house.mtl");
	//meshList[GEO_ABANDONEDHOUSE]->textureID = LoadTGA("Images//abandonedhouseBaseColor.tga");

	{
		// BUMPER CAR ITEMS
		meshList[BUMPERCAR] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//Bumper Car//bumper_car.obj", "Models//Bumper Car//bumper_car.mtl");
		meshList[BUMPERCAR]->textureID = LoadTGA("Images//Bumper Car//bumpercar_baseColor.tga");

		meshList[BUMPERCARTENT] = MeshBuilder::GenerateOBJMTL("bumper car tent", "Models//Bumper Car//bumpercar_tent.obj", "Models//Bumper Car//bumpercar_tent.mtl");
		meshList[BUMPERCARTENT]->textureID = LoadTGA("Images//Bumper Car//bumpercar_tent.tga");
	}

	//meshList[TALLTREE] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//tree//VeryTallTree.obj", "Models//tree//VeryTallTree.mtl");
	//meshList[TALLTREE]->textureID = LoadTGA("Images//TallTree_baseColor.tga");

	meshList[FOREST] = MeshBuilder::GenerateOBJMTL("bumper car", "Models//forest//forest.obj", "Models//forest//forest.mtl");
	meshList[FOREST]->textureID = LoadTGA("Images//forest//forest_baseColor.tga");

	meshList[CLOWN] = MeshBuilder::GenerateOBJMTL("Clown", "Models//clown.obj", "Models//clown.mtl");
	meshList[CLOWN]->textureID = LoadTGA("Images//clown.tga");

	//meshList[EXITBUTTON] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	//meshList[EXITBUTTON]->textureID = LoadTGA("Images//exitScene01button.tga");

	meshList[PAUSEMENU] = MeshBuilder::GenerateQuad("pause", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[PAUSEMENU]->textureID = LoadTGA("Images//scene01pausemenuv2.tga");

	meshList[MAINPAUSE] = MeshBuilder::GenerateQuad("pause", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[MAINPAUSE]->textureID = LoadTGA("Images//scene01 UI//mainpausemenu.tga");

	meshList[ENTERBUMPERCARGAMEPROMPT] = MeshBuilder::GenerateQuad("enter", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[ENTERBUMPERCARGAMEPROMPT]->textureID = LoadTGA("Images//scene01 UI//bumpercargameprompt.tga");

	meshList[EXITBUMPERCARGAMEPROMPT] = MeshBuilder::GenerateQuad("exit", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[EXITBUMPERCARGAMEPROMPT]->textureID = LoadTGA("Images//scene01 UI//exitbumpercargameprompt.tga");

	meshList[ENTERBASKETBALLPROMPT] = MeshBuilder::GenerateQuad("enter goat", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[ENTERBASKETBALLPROMPT]->textureID = LoadTGA("Images//scene01 UI//basketballgameprompt.tga");

	meshList[ENTERBALLBOUNCERPROMPT] = MeshBuilder::GenerateQuad("enter bounce", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[ENTERBALLBOUNCERPROMPT]->textureID = LoadTGA("Images//scene01 UI//ballbouncergameprompt.tga");

	meshList[ENTERDUCKSHOOTINGPROMPT] = MeshBuilder::GenerateQuad("enter duck", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[ENTERDUCKSHOOTINGPROMPT]->textureID = LoadTGA("Images//scene01 UI//duckshootinggameprompt.tga");

	{
		//meshList[GEO_DEER] = MeshBuilder::GenerateOBJMTL("demon", "Models//model_containment//obj//13573_Musk_Deer_v1_L3.obj", "Models//model_containment//mtl//13573_Musk_Deer_v1_L3.mtl");
		//meshList[GEO_DEER]->textureID = LoadTGA("Models//model_containment//textures//musk_deer.tga");

		meshList[GEO_COW] = MeshBuilder::GenerateOBJMTL("lowkeychillguy", "Models//model_containment//obj//cow.obj", "Models//model_containment//mtl//cow.mtl");
		meshList[GEO_COW]->textureID = LoadTGA("Images//model_containment//textures//cow.tga");

		//meshList[GEO_SHEEP] = MeshBuilder::GenerateOBJMTL("demon", "Models//model_containment//obj//13574_Marco_Polo_Sheep_v1_L3.obj", "Models//model_containment//mtl//13574_Marco_Polo_Sheep_v1_L3.mtl");
		//meshList[GEO_SHEEP]->textureID = LoadTGA("Models//model_containment//textures//13574_Marco_Polo_Diffuse.tga");
	}

	{
		// LOW POLY BUILDING
		meshList[LOWPOLYBUILDING] = MeshBuilder::GenerateOBJMTL("building", "Models//low poly building//low_poly_building.obj", "Models//low poly building//low_poly_building.mtl");
		meshList[LOWPOLYBUILDING]->textureID = LoadTGA("Images//low poly building//lowpolybuilding_baseColor.tga");
	}

	{
		// PLAYER INDICATOR
		//meshList[PLAYER1INDICATORUI] = MeshBuilder::GenerateQuad("player1", glm::vec3(1.f, 1.f, 1.f), 1.f);
		//meshList[PLAYER1INDICATORUI]->textureID = LoadTGA("Images//scene01 UI//scene01player1_indicatorUI.tga");

		//meshList[PLAYER2INDICATORUI] = MeshBuilder::GenerateQuad("player2", glm::vec3(1.f, 1.f, 1.f), 1.f);
		//meshList[PLAYER2INDICATORUI]->textureID = LoadTGA("Images//scene01 UI//scene01player2_indicatorUI.tga");
	}

	{
		// GROUND
		meshList[GREYGROUND] = MeshBuilder::GenerateQuad("grey ground", glm::vec3(1.f, 1.f, 1.f), 1.f);
		meshList[GREYGROUND]->textureID = LoadTGA("Images//scene01_ground//greyground.tga");
	}

	{
		// LOW POLY TREE
		meshList[BIRCHTREE] = MeshBuilder::GenerateOBJMTL("tree", "Models//birchtree//birchtree.obj", "Models//birchtree/birchtree.mtl");
		meshList[BIRCHTREE]->textureID = LoadTGA("Images//birchtree/birchtree_baseColor.tga");
	}

	{
		// LOW POLY FENCE
		meshList[CARTOONFENCE] = MeshBuilder::GenerateOBJMTL("cartoonfence", "Models//low poly fence//low_poly_fence.obj", "Models//low poly fence//low_poly_fence.mtl");
		meshList[CARTOONFENCE]->textureID = LoadTGA("Images//low poly fence//lowpolyfence.tga");
	}

	{
		// CARNIVAL TENT
		meshList[CARNIVALTENT] = MeshBuilder::GenerateOBJMTL("carnival tent", "Models//tent//carnivaltent.obj", "Models//tent//carnivaltent.mtl");

		meshList[ALVINTENT] = MeshBuilder::GenerateOBJMTL("CarnivalTent", "Models//alvintent.obj", "Models//alvintent.mtl");
		meshList[ALVINTENT]->textureID = LoadTGA("Images//alvintent.tga");
	}

	{
		// BASKETBALL COURT
		meshList[BASKETBALLCOURT] = MeshBuilder::GenerateOBJMTL("basketball court", "Models//scene01 basketball court//basketball_court_low_poly_purple.obj", "Models//scene01 basketball court//basketball_court_low_poly_purple.mtl");
		meshList[BASKETBALLCOURT]->textureID = LoadTGA("Images//scene01 basketball court//scene01 basketballcourt.tga");
	}

	{
		// SCENE02 TENT
		meshList[GEO_TENT] = MeshBuilder::GenerateOBJ("Tent", "Models//DuckShoot//Tents.obj");
		meshList[GEO_TENT]->textureID = LoadTGA("Images//DSTent.tga");
	}

	{
		// MCDONALDS
		//meshList[MCDONALDS] = MeshBuilder::GenerateOBJMTL("mcd", "Models//mcdonalds//mcdonalds.obj", "Models//mcdonalds//mcdonalds.mtl");
		//meshList[MCDONALDS]->textureID = LoadTGA("Images//mcondalds//mcdonalds_baseColor2.tga");
	}

	{
		// CUTE CHARACTER
		//meshList[CUTECHARACTER] = MeshBuilder::GenerateOBJMTL("cutre", "Models//cute character//cute_character.obj", "Models//cute character//cute_character.mtl");
	}

	{
		// CONCERT
		meshList[CONCERTSTAGE] = MeshBuilder::GenerateOBJMTL("concert", "Models//concert stage//simple_concert_stage.obj", "Models//concert stage//simple_concert_stage.mtl");
		meshList[CONCERTSTAGE]->textureID = LoadTGA("Images//concert stage//Scene_-_Root_baseColor copy.tga");
	}

	// Setup invisible fence zones (AABB) using the coordinates provided
	// Keep Y from -1 to 5 as requested
	{
		fenceZones.clear();
		const float yMin = -1.0f;
		const float yMax = 5.0f;
		const float thickness = 4.0f; // thickness around line/segment to make an AABB

		auto addSegmentAABB = [&](float x1, float z1, float x2, float z2)
			{
				float minX = (std::min)(x1, x2) - thickness * 0.5f;
				float maxX = (std::max)(x1, x2) + thickness * 0.5f;
				float minZ = (std::min)(z1, z2) - thickness * 0.5f;
				float maxZ = (std::max)(z1, z2) + thickness * 0.5f;
				fenceZones.emplace_back(glm::vec3(minX, yMin, minZ), glm::vec3(maxX, yMax, maxZ));
			};

		// Segments requested:
		// 1) (x -21, z -41) to (x -21, z -111)
		addSegmentAABB(-21.0f, -41.0f, -21.0f, -111.0f);

		// 2) (x -21, z -111) to (x -76, z -111)
		addSegmentAABB(-21.0f, -111.0f, 76.0f, -111.0f);

		// 3) (x 76, z -111) to (x 76, z -42)
		addSegmentAABB(76.0f, -111.0f, 76.0f, -42.0f);

		// 4) (x 76, z -42) to (x 35, z -41) -- diagonal, AABB will cover bounding rectangle
		addSegmentAABB(76.0f, -42.0f, 35.0f, -41.0f);

		// 5) (x 20, z -41) to (x -21, z -41)
		addSegmentAABB(20.0f, -41.0f, -21.0f, -41.0f);
	}

	{
		concertStageZone.clear();
		const float yMin = -1.0f;
		const float yMax = 50.0f;
		const float thickness = 4.0f; // thickness around line/segment to make an AABB

		auto addSegmentAABB = [&](float x1, float z1, float x2, float z2)
			{
				float minX = (std::min)(x1, x2) - thickness * 0.5f;
				float maxX = (std::max)(x1, x2) + thickness * 0.5f;
				float minZ = (std::min)(z1, z2) - thickness * 0.5f;
				float maxZ = (std::max)(z1, z2) + thickness * 0.5f;
				fenceZones.emplace_back(glm::vec3(minX, yMin, minZ), glm::vec3(maxX, yMax, maxZ));
			};

		// Segments requested:
		// (x, z) to (x, z)
		addSegmentAABB(106, 40, 106, -10);
		addSegmentAABB(106, -10, 136, -10);
		addSegmentAABB(136, -10, 136, 40);
		addSegmentAABB(136, 40, 106, 40);
	}

	{
		basketballBuildingZone.clear();
		const float yMin = -1.0f;
		const float yMax = 50.0f;
		const float thickness = 4.0f; // thickness around line/segment to make an AABB

		auto addSegmentAABB = [&](float x1, float z1, float x2, float z2)
			{
				float minX = (std::min)(x1, x2) - thickness * 0.5f;
				float maxX = (std::max)(x1, x2) + thickness * 0.5f;
				float minZ = (std::min)(z1, z2) - thickness * 0.5f;
				float maxZ = (std::max)(z1, z2) + thickness * 0.5f;
				fenceZones.emplace_back(glm::vec3(minX, yMin, minZ), glm::vec3(maxX, yMax, maxZ));
			};

		// Segments requested:
		// (x, z) to (x, z)
		addSegmentAABB(143, 72, 132, 72);
		addSegmentAABB(93, 75, 79, 75);
	}

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);


	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(-200, 150, 0);
	light[0].color = glm::vec3(1, 1, 1);
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

void Scene01::UpdateObjectMovement(double dt)
{
	if (!BumperCarGameEntered)
	{
		// Forward
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT))
			objectPos.z -= moveSpeed * dt;

		// Backward
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT))
			objectPos.z += moveSpeed * dt;

		// Left
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_DOWN))
			objectPos.x -= moveSpeed * dt;

		// Right
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP))
			objectPos.x += moveSpeed * dt;

		// Up
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_I))
			objectPos.y += moveSpeed * dt;

		// Down
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_K))
			objectPos.y -= moveSpeed * dt;

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_LEFT_ALT))
		{
			// Print position
			std::cout << "Object Position: ("
				<< objectPos.x << ", "
				<< objectPos.y << ", "
				<< objectPos.z << ")"
				<< std::endl;
		}
	}
}

void Scene01::UpdateIntro(double dt)
{
	introTimer += static_cast<float>(dt);

	if (!introReturning)
	{
		float t = introTimer / introDuration;

		if (t >= 1.0f)
		{
			t = 1.0f;
			introReturning = true;
			introTimer = 0.0f;
		}

		// Smooth movement (ease in/out)
		float smoothT = t * t * (3.f - 2.f * t);

		camera1.position = glm::mix(introStartPos, introEndPos, smoothT);
		camera1.target = camera1.position + glm::vec3(0.9, 0.1, -0.8);
	}
	else
	{
		float t = introTimer / introReturnDuration;

		if (t >= 1.0f)
		{
			t = 1.0f;
			introActive = false;  // intro finished
		}

		float smoothT = t * t * (3.f - 2.f * t);

		camera1.position = glm::mix(introEndPos, introStartPos, smoothT);
		camera1.target = camera1.position + glm::vec3(0.9, 0.1, -0.8);
	}
}

void Scene01::Update(double dt)
{
	if (introActive)
	{
		UpdateIntro(dt);
		return; // skip normal game update
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

	if (!BumperCarGameEntered)
	{
		player1InCar = false;
		player2InCar = false;
	}

	if (!player1InCar)
	{
		// store previous position in case movement code somehow ends inside fence
		glm::vec3 prevPos1 = camera1.position;
		glm::vec3 prevTarget1 = camera1.target;

		// Prevent camera from going below ground before input
		if (camera1.position.y < 3.0f) {
			camera1.position.y = 3.0f;
			if (camera1.target.y < 3.0f)
				camera1.target.y = 3.0f;
			camera1.Init(camera1.position, camera1.target, camera1.up);
		}

		HandleKeyPress1(camera1, dt);
		HandleMouseInput(camera1);

		// Safety: revert if we somehow ended inside a fence
		if (IsInsideFence(camera1.position))
		{
			camera1.position = prevPos1;
			camera1.target = prevTarget1;
			cameraVelocity1 = glm::vec3(0.0f);
		}
	}

	if (BumperCarGameEntered)
	{
		if (!player2InCar)
		{
			glm::vec3 prevPos2 = camera2.position;
			glm::vec3 prevTarget2 = camera2.target;

			// Prevent camera from going below ground before input
			if (camera2.position.y < 3.0f) {
				camera2.position.y = 3.0f;
				if (camera2.target.y < 3.0f)
					camera2.target.y = 3.0f;
				camera2.Init(camera2.position, camera2.target, camera2.up);
			}

			HandleKeyPress2(camera2, dt);

			// Safety: revert if we somehow ended inside a fence
			if (IsInsideFence(camera2.position))
			{
				camera2.position = prevPos2;
				camera2.target = prevTarget2;
				cameraVelocity2 = glm::vec3(0.0f);
			}
		}
	}

	if (player1InCar)
	{
		glm::vec3 prevPos1 = camera1.position;
		glm::vec3 prevTarget1 = camera1.target;

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

			// Apply simple damping (friction) so cars slow after bouncing
			float damp = std::exp(-linearDamping * static_cast<float>(dt));
			cameraVelocity1 *= damp;
			cameraVelocity2 *= damp;
		}

		for (const auto& box : fenceZones)
		{
			const glm::vec3& mn = box.first;
			const glm::vec3& mx = box.second;
			if (camera1.position.x >= mn.x && camera1.position.x <= mx.x &&
				camera1.position.y >= mn.y && camera1.position.y <= mx.y &&
				camera1.position.z >= mn.z && camera1.position.z <= mx.z)
			{
				// Distances to each face
				float dxMin = camera1.position.x - mn.x;
				float dxMax = mx.x - camera1.position.x;
				float dzMin = camera1.position.z - mn.z;
				float dzMax = mx.z - camera1.position.z;

				// Find smallest penetration and associated normal (XZ-plane only)
				float minDist = dxMin;
				glm::vec3 normal(-1.0f, 0.0f, 0.0f);

				if (dxMax < minDist) { minDist = dxMax; normal = glm::vec3(1.0f, 0.0f, 0.0f); }
				if (dzMin < minDist) { minDist = dzMin; normal = glm::vec3(0.0f, 0.0f, -1.0f); }
				if (dzMax < minDist) { minDist = dzMax; normal = glm::vec3(0.0f, 0.0f, 1.0f); }

				const float eps = 0.01f; // small offset to avoid re-penetration due to float error
				glm::vec3 push = normal * (minDist + eps);

				// Push camera out of the fence and move its look target the same amount
				camera1.position += push;
				camera1.target += push;

				// Reflect velocity across the collision normal (XZ only), apply restitution and slight tangential damping
				glm::vec3 v = cameraVelocity1;
				glm::vec3 n = glm::vec3(normal.x, 0.0f, normal.z);
				if (glm::length(n) > 0.0f)
				{
					n = glm::normalize(n);
					float restitution = 0.6f; // 0 = no bounce, 1 = perfect elastic
					float vn = glm::dot(v, n);
					// only reflect if moving into the wall
					if (vn < 0.0f)
					{
						v = v - (1.0f + restitution) * vn * n;
						// minor damping to avoid perpetual bouncing
						v *= 0.9f;
					}
					else
					{
						// small damping if somehow inside but moving away
						v *= 0.95f;
					}
					cameraVelocity1 = v;
				}
				else
				{
					// fallback: stop movement on penetration
					cameraVelocity1 = glm::vec3(0.0f);
				}
				break; // handled one fence, exit
			}
		}

		// Prevent camera from going below ground after camera updates
		if (camera1.position.y < 3.0f) {
			camera1.position.y = 3.0f;
			if (camera1.target.y < 3.0f)
				camera1.target.y = 3.0f;
			camera1.Init(camera1.position, camera1.target, camera1.up);
		}
	}

	if (BumperCarGameEntered)
	{
		if (player2InCar)
		{
			glm::vec3 prevPos2 = camera2.position;
			glm::vec3 prevTarget2 = camera2.target;

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

				// Apply simple damping (friction) so cars slow after bouncing
				float damp = std::exp(-linearDamping * static_cast<float>(dt));
				cameraVelocity1 *= damp;
				cameraVelocity2 *= damp;
			}

			for (const auto& box : fenceZones)
			{
				const glm::vec3& mn = box.first;
				const glm::vec3& mx = box.second;
				if (camera2.position.x >= mn.x && camera2.position.x <= mx.x &&
					camera2.position.y >= mn.y && camera2.position.y <= mx.y &&
					camera2.position.z >= mn.z && camera2.position.z <= mx.z)
				{
					// Distances to each face
					float dxMin = camera2.position.x - mn.x;
					float dxMax = mx.x - camera2.position.x;
					float dzMin = camera2.position.z - mn.z;
					float dzMax = mx.z - camera2.position.z;

					// Find smallest penetration and associated normal (XZ-plane only)
					float minDist = dxMin;
					glm::vec3 normal(-1.0f, 0.0f, 0.0f);

					if (dxMax < minDist) { minDist = dxMax; normal = glm::vec3(1.0f, 0.0f, 0.0f); }
					if (dzMin < minDist) { minDist = dzMin; normal = glm::vec3(0.0f, 0.0f, -1.0f); }
					if (dzMax < minDist) { minDist = dzMax; normal = glm::vec3(0.0f, 0.0f, 1.0f); }

					const float eps = 0.01f;
					glm::vec3 push = normal * (minDist + eps);

					// Push camera out of the fence and move its look target the same amount
					camera2.position += push;
					camera2.target += push;

					// Reflect velocity across the collision normal (XZ only), apply restitution and slight tangential damping
					glm::vec3 v = cameraVelocity2;
					glm::vec3 n = glm::vec3(normal.x, 0.0f, normal.z);
					if (glm::length(n) > 0.0f)
					{
						n = glm::normalize(n);
						float restitution = 0.6f;
						float vn = glm::dot(v, n);
						if (vn < 0.0f)
						{
							v = v - (1.0f + restitution) * vn * n;
							v *= 0.9f;
						}
						else
						{
							v *= 0.95f;
						}
						cameraVelocity2 = v;
					}
					else
					{
						cameraVelocity2 = glm::vec3(0.0f);
					}
					break;
				}
			}

			if (camera2.position.y < 3.0f) {
				camera2.position.y = 3.0f;
				if (camera2.target.y < 3.0f)
					camera2.target.y = 3.0f;
				camera2.Init(camera2.position, camera2.target, camera2.up);
			}
		}
	}

	if (player1InCar && player2InCar)
	{
		// Camera radius for collision detection
		const float cameraRadius = 3.5f;
		// Minimum separation on XZ plane (sum of radii)
		const float minSeparation = cameraRadius * 2.0f;
		glm::vec3 diff = camera2.position - camera1.position;
		diff.y = 0.0f; // resolve only in XZ plane
		float dist = glm::length(diff);

		if (dist < 0.0001f)
		{
			// avoid zero-length vector
			diff = glm::vec3(1.0f, 0.0f, 0.0f);
			dist = 1.0f;
		}

		if (dist < minSeparation)
		{
			float overlap = (minSeparation - dist) + 0.01f; // small epsilon to prevent re-penetration
			glm::vec3 n = glm::normalize(diff);

			// Push each camera half the overlap away
			glm::vec3 push = n * (overlap * 0.5f);
			camera1.position -= push;
			camera2.position += push;
			camera1.target -= push;
			camera2.target += push;

			// Simple 1D collision along n: exchange/reflect normal components with restitution
			float restitution = 0.6f;
			float v1n = glm::dot(cameraVelocity1, n);
			float v2n = glm::dot(cameraVelocity2, n);

			// masses = 1 assumed; compute post-collision scalar normal velocities
			float u1 = v1n;
			float u2 = v2n;
			float newV1n = (u1 * (1.0f - restitution) + u2 * (1.0f + restitution)) * 0.5f;
			float newV2n = (u2 * (1.0f - restitution) + u1 * (1.0f + restitution)) * 0.5f;

			cameraVelocity1 += (newV1n - v1n) * n;
			cameraVelocity2 += (newV2n - v2n) * n;

			// Slight tangential damping to avoid sliding forever
			glm::vec3 t1 = cameraVelocity1 - glm::dot(cameraVelocity1, n) * n;
			glm::vec3 t2 = cameraVelocity2 - glm::dot(cameraVelocity2, n) * n;
			cameraVelocity1 = glm::dot(cameraVelocity1, n) * n + t1 * 0.95f;
			cameraVelocity2 = glm::dot(cameraVelocity2, n) * n + t2 * 0.95f;
		}
	}

	{
		const float groundY = 3.3f;

		// Player1 vertical integration when not in car
		if (!player1InCar)
		{
			if (isJumping1 || cameraVerticalVel1 != 0.0f)
			{
				// integrate
				cameraVerticalVel1 += gravity * static_cast<float>(dt);
				float deltaY = cameraVerticalVel1 * static_cast<float>(dt);

				camera1.position.y += deltaY;
				camera1.target.y += deltaY;

				// Land check
				if (camera1.position.y <= groundY)
				{
					camera1.position.y = groundY;
					cameraVerticalVel1 = 0.0f;
					isJumping1 = false;

					// Recompute look direction from stored azimuth/altitude to avoid abrupt pitch/target snaps
					float az = glm::radians(camera1.azimuth);
					float alt = glm::radians(camera1.altitude);
					glm::vec3 dir;
					dir.x = cosf(alt) * cosf(az);
					dir.y = sinf(alt);
					dir.z = cosf(alt) * sinf(az);
					camera1.target = camera1.position + glm::normalize(dir);

					// Refresh camera internals
					camera1.Init(camera1.position, camera1.target, camera1.up);
				}
			}
		}

		// Player2 vertical integration when not in car
		if (!player2InCar)
		{
			if (isJumping2 || cameraVerticalVel2 != 0.0f)
			{
				// integrate
				cameraVerticalVel2 += gravity * static_cast<float>(dt);
				float deltaY2 = cameraVerticalVel2 * static_cast<float>(dt);

				camera2.position.y += deltaY2;
				camera2.target.y += deltaY2;

				// Land check
				if (camera2.position.y <= groundY)
				{
					camera2.position.y = groundY;
					cameraVerticalVel2 = 0.0f;
					isJumping2 = false;

					// Recompute look direction from stored azimuth/altitude to avoid abrupt pitch/target snaps
					float az = glm::radians(camera2.azimuth);
					float alt = glm::radians(camera2.altitude);
					glm::vec3 dir;
					dir.x = cosf(alt) * cosf(az);
					dir.y = sinf(alt);
					dir.z = cosf(alt) * sinf(az);
					camera2.target = camera2.position + glm::normalize(dir);

					// Refresh camera internals
					camera2.Init(camera2.position, camera2.target, camera2.up);
				}
			}
		}
	}

	float temp = 1.f / dt;
	fps = glm::round(temp * 100.f) / 100.f;

	// Output camera positions for debugging (Scene01 only)
	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		std::cout << std::fixed << std::setprecision(2)
			<< "Camera1 Pos: (" << camera1.position.x << ", " << camera1.position.y << ", " << camera1.position.z << ")  "
			<< "Camera2 Pos: (" << camera2.position.x << ", " << camera2.position.y << ", " << camera2.position.z << ")  "
			<< "FPS: " << fps << std::endl;
	}

	{
		// ENTER BUMPER CAR PROMPT
		const glm::vec3 minPos(-22, -1, -115);
		const glm::vec3 maxPos(76, 6, -40);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		bool inside = isInsideBox(camera1.position, minPos, maxPos);

		// RESET PROMPTS FIRST
		EnterBumperCarGamePrompt = false;
		ExitBumperCarGamePrompt = false;

		// ENTER LOGIC
		if (!BumperCarGameEntered && inside)
		{
			EnterBumperCarGamePrompt = true;

			if (EnterBumperCarGamePrompt)
			{
				//PlaySound(TEXT("Sounds//paper-ripping.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}

			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_ENTER))
			{
				BumperCarGameEntered = true;

				// OPTIONAL: teleport inside arena
				camera1.position = glm::vec3(26, 3.3, -51);
			}
			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F1))
			{
				BumperCarGameEntered = false;

				// TELEPORT OUTSIDE ARENA
				camera1.position = glm::vec3(27.f, 3.3f, -29.f);
			}
		}

		// EXIT LOGIC
		if (BumperCarGameEntered && !inside)
		{
			ExitBumperCarGamePrompt = true;
			if (ExitBumperCarGamePrompt)
			{
				//PlaySound(TEXT("Sounds//paper-ripping.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}

			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_ENTER))
			{
				BumperCarGameEntered = false;
				player1InCar = false;
				cameraVelocity1 =glm::vec3(0.0f);

				// TELEPORT PLAYER OUTSIDE
				camera1.position = glm::vec3(27.f, 3.3f, -29.f);
			}
			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F1))
			{
				BumperCarGameEntered = true;

				// KEEP PLAYER INSIDE ARENA
				camera1.position = glm::vec3(26, 3.3, -51);
			}
		}
	}

	{
		// ENTER BASKETBALL PROMPT

		const glm::vec3 minPos(94, -1, 72);
		const glm::vec3 maxPos(127, 6, 122);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		bool inside = isInsideBox(camera1.position, minPos, maxPos);

		// RESET PROMPTS FIRST
		EnterBasketballGamePrompt = false;

		// ENTER LOGIC
		if (!BasketballGameEntered && inside)
		{
			EnterBasketballGamePrompt = true;
			if (EnterBasketballGamePrompt)
			{
				//PlaySound(TEXT("Sounds//paper-ripping.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}

			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_ENTER))
			{
				BasketballGameEntered = true;
				scene03request = true;
			}
			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F1))
			{
				BasketballGameEntered = false;

				// TELEPORT OUTSIDE ARENA
				camera1.position = glm::vec3(112, 3.3, 62);
			}
		}
	}

	{
		// ENTER BALL BOUNCER PROMPT

		const glm::vec3 minPos(92, -1, -75);
		const glm::vec3 maxPos(141, 6, -25);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		bool inside = isInsideBox(camera1.position, minPos, maxPos);

		// RESET PROMPTS FIRST
		EnterBallBouncerGamePrompt = false;

		// ENTER LOGIC
		if (!BallBouncerGameEntered && inside)
		{
			EnterBallBouncerGamePrompt = true;
			if (EnterBallBouncerGamePrompt)
			{
				//PlaySound(TEXT("Sounds//paper-ripping.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}

			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_ENTER))
			{
				BallBouncerGameEntered = true;
				scene04request = true;
			}
			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F1))
			{
				BallBouncerGameEntered = false;

				// TELEPORT OUTSIDE ARENA
				camera1.position = glm::vec3(85, 3.3, -22);
			}
		}
	}

	{
		// ENTER DUCK SHOOTING PROMPT

		const glm::vec3 minPos(1, -1, 161);
		const glm::vec3 maxPos(22, 6, 180);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		bool inside = isInsideBox(camera1.position, minPos, maxPos);

		// RESET PROMPTS FIRST
		EnterDuckShootingGamePrompt = false;

		// ENTER LOGIC
		if (!DuckShootingGameEntered && inside)
		{
			EnterDuckShootingGamePrompt = true;
			if (EnterDuckShootingGamePrompt)
			{
				//PlaySound(TEXT("Sounds//paper-ripping.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}

			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_ENTER))
			{
				DuckShootingGameEntered = true;
				scene02request = true;
			}
			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F1))
			{
				DuckShootingGameEntered = false;

				// TELEPORT OUTSIDE ARENA
				camera1.position = glm::vec3(10, 3.3, 150);
			}
		}
	}

	// SONG DATA
	static std::vector<std::wstring> songs = {
		L"Sounds//8DAUDIO_Disfigure_Blank.wav",
		L"F.O.O.L_Criminals.wav",
		L"Sounds//roblox.wav"
	};

	static std::vector<int> songDurations = {
		199,
		251,
		60
	};

	static size_t currentSongIndex = 0;
	static bool isPlaying = false;
	static std::chrono::steady_clock::time_point playStartTime;


	// ENTER CONCERT ZONE
	const glm::vec3 minPos(80, -1, -8);
	const glm::vec3 maxPos(135, 6, 42);

	auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
		return (p.x >= mn.x && p.x <= mx.x) &&
			(p.y >= mn.y && p.y <= mx.y) &&
			(p.z >= mn.z && p.z <= mx.z);
		};

	bool inside = isInsideBox(camera1.position, minPos, maxPos);

	if (inside)
	{
		if (!isPlaying)
		{
			// Start first song
			PlaySound(songs[currentSongIndex].c_str(), NULL, SND_FILENAME | SND_ASYNC);
			playStartTime = std::chrono::steady_clock::now();
			isPlaying = true;

			std::cout << "Started song " << currentSongIndex << "\n";
		}
		else
		{
			auto now = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - playStartTime).count();

			// Check duration of CURRENT song
			if (elapsed >= songDurations[currentSongIndex])
			{
				// Stop current
				PlaySound(NULL, 0, 0);

				// Move to next
				currentSongIndex = (currentSongIndex + 1) % songs.size();

				// Play next
				PlaySound(songs[currentSongIndex].c_str(), NULL, SND_FILENAME | SND_ASYNC);

				playStartTime = now;

				std::cout << "Switched to song " << currentSongIndex << "\n";
			}
		}
	}
	else
	{
		if (isPlaying)
		{
			PlaySound(NULL, 0, 0);
			isPlaying = false;

			std::cout << "Stopped playing\n";
		}
	}

	UpdateObjectMovement(dt);
}

void Scene01::RenderSkybox()
{
	// Front face (no rotation needed if quad faces -Z by default)
	modelStack.PushMatrix();
	modelStack.Translate(0.f, 0.f, -500.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	//modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	RenderMesh(meshList[GEO_FRONT], false);
	modelStack.PopMatrix();

	// Back face (rotate 180 degrees around Y)
	modelStack.PushMatrix();
	modelStack.Translate(0.f, 0.f, 500.f);
	modelStack.Rotate(180.f, 0, 1.f, 0.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	RenderMesh(meshList[GEO_BACK], false);
	modelStack.PopMatrix();

	// Left face (rotate 90 degrees around Y)
	modelStack.PushMatrix();
	modelStack.Translate(-500.f, 0.f, 0.f);
	modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
	//modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	RenderMesh(meshList[GEO_LEFT], false);
	modelStack.PopMatrix();

	// Right face (rotate -90 degrees around Y)
	modelStack.PushMatrix();
	modelStack.Translate(500.f, 0.f, 0.f);
	modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
	//modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	RenderMesh(meshList[GEO_RIGHT], false);
	modelStack.PopMatrix();

	// Top face (rotate -90 degrees around X)
	modelStack.PushMatrix();
	modelStack.Translate(0.f, 500.f, 0.f);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
	//modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	modelStack.Scale(10.f, 10.f, 10.f);
	RenderMesh(meshList[GEO_TOP], false);
	modelStack.PopMatrix();

	// Bottom face (rotate 90 degrees around X)
	modelStack.PushMatrix();
	modelStack.Translate(0.f, -500.f, 0.f);
	modelStack.Scale(10.f, 10.f, 10.f); // CHANGE TO 10
	modelStack.Rotate(-90.f, 1.f, 0.f, 0.f);
	//modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
	RenderMesh(meshList[GEO_BOTTOM], false);
	modelStack.PopMatrix();
}

void Scene01::RenderPathway()
{
	// Render the pathway as a smooth curved strip built from identical GREYGROUND quads.
	// Each tile keeps the same scale used previously so visual scale remains unchanged.
	const int segments = 64; // increase for a smoother curve
	const glm::vec3 p0(-100.f, 0.3f, 25.f);  // start
	const glm::vec3 p1(-40.f, 0.3f, 60.f);   // control 1
	const glm::vec3 p2(40.f, 0.3f, -10.f);   // control 2
	const glm::vec3 p3(100.f, 0.3f, 25.f);   // end

	auto cubicBezier = [](float t, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
		float u = 1.0f - t;
		return u * u * u * a + 3.0f * u * u * t * b + 3.0f * u * t * t * c + t * t * t * d;
		};
	auto cubicBezierDeriv = [](float t, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
		float u = 1.0f - t;
		return 3.0f * u * u * (b - a) + 6.0f * u * t * (c - b) + 3.0f * t * t * (d - c);
		};

	// Material for the path (keeps same visual as before)
	meshList[GREYGROUND]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	meshList[GREYGROUND]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
	meshList[GREYGROUND]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GREYGROUND]->material.kShininess = 5.0f;

	// Tweak this to change path width. Original X-scale was 100.f — reduce to make path narrower.
	const float pathWidth = 40;    // <-- adjust this value
	const float pathHeightScale = 50;
	const float pathDepthScale = 5;

	for (int i = 0; i < segments; ++i)
	{
		// center each segment on its parametric midpoint for nicer overlap
		float t = (i + 0.5f) / static_cast<float>(segments);
		glm::vec3 pos = cubicBezier(t, p0, p1, p2, p3);
		glm::vec3 tangent = cubicBezierDeriv(t, p0, p1, p2, p3);

		// Compute yaw so the quad aligns with the path tangent (XZ plane)
		float yaw = glm::degrees(atan2(tangent.z, tangent.x));

		modelStack.PushMatrix();
		// Position on the curve
		modelStack.Translate(pos.x, pos.y, pos.z);

		// Align tile along curve's heading
		modelStack.Rotate(yaw, 0.f, 1.f, 0.f);

		// Orient the quad flat on the ground and flip to match original orientation
		modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
		modelStack.Rotate(180.f, 1.f, 0.f, 0.f);

		// Keep tile scale but use narrower width (X)
		modelStack.Scale(pathWidth, pathHeightScale, pathDepthScale);

		// Render without lighting (same as original pathway)
		RenderMesh(meshList[GREYGROUND], false);
		modelStack.PopMatrix();
	}
}

void Scene01::RenderGamePathways()
{
	//	PATH TO BUMPER CARS
	modelStack.PushMatrix();
	modelStack.Translate(26.5, 0.3, -20);
	modelStack.Scale(20, 0.1, 45);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
	modelStack.Rotate(180.f, 1.f, 0.f, 0.f);
	meshList[GREYGROUND]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	meshList[GREYGROUND]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
	meshList[GREYGROUND]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GREYGROUND]->material.kShininess = 5.0f;
	RenderMesh(meshList[GREYGROUND], false);
	modelStack.PopMatrix();

	// PATH TO BASKETBALL
	modelStack.PushMatrix();
	modelStack.Translate(113, 0.3, 49);
	modelStack.Scale(20, 0.1, 45);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
	modelStack.Rotate(180.f, 1.f, 0.f, 0.f);
	meshList[GREYGROUND]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	meshList[GREYGROUND]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
	meshList[GREYGROUND]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GREYGROUND]->material.kShininess = 5.0f;
	RenderMesh(meshList[GREYGROUND], false);
	modelStack.PopMatrix();

	// PATH TO DUCK SHOOTING
	modelStack.PushMatrix();
	modelStack.Translate(15, 0.3, 100);
	modelStack.Scale(22, 0.1, 116);
	modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
	modelStack.Rotate(180.f, 1.f, 0.f, 0.f);
	meshList[GREYGROUND]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	meshList[GREYGROUND]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
	meshList[GREYGROUND]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	meshList[GREYGROUND]->material.kShininess = 5.0f;
	RenderMesh(meshList[GREYGROUND], false);
	modelStack.PopMatrix();
}

void Scene01::RenderBirchTrees()
{
	// positions from the original explicit list
	static const std::vector<glm::vec3> positions = {
		{30,0,53},{48,0,66},{65,0,69},{66,0,83},{55,0,92},{44,0,109},
		{52,0,117},{67,0,120},{74,0,130},{71,0,140},{60,0,146},{52,0,137},
		{41,0,143},{34,0,135},{1,0,153},{0,0,144},{0,0,128},{-4,0,112},
		{0,0,98},{-6,0,86},{-14,0,92},{-15,0,124},{-16,0,107},{-18,0,136},
		{-14,0,151}, {-23, 0, -10}, {-30, 0, -10}, {-42, 0, -8}, {-50, 0, -16}, {-42, 0, -23}, {-26, 0, -26}, {3, 0, -18},
		{41, 0, -14}, {43, 0, -22}, {51, 0, -31}, {63, 0, -34}, {62, 0, -24}, {70, 0, -18}
	};

	const float cullRadius = 300.f;
	const float cullRadius2 = cullRadius * cullRadius;
	glm::vec3 camPos = viewStack.Top() * glm::vec4(0, 0, 0, 1);

	// set material once
	meshList[BIRCHTREE]->material.kAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
	meshList[BIRCHTREE]->material.kDiffuse = glm::vec3(0.8f, 0.9f, 0.7f);  // leaf green tint
	meshList[BIRCHTREE]->material.kSpecular = glm::vec3(0.3f, 0.3f, 0.3f);  // subtle highlights
	meshList[BIRCHTREE]->material.kShininess = 15.0f;

	for (const auto& pos : positions)
	{
		float dist2 = glm::dot(pos - camPos, pos - camPos);
		if (dist2 > cullRadius2) continue;

		modelStack.PushMatrix();
		modelStack.Translate(pos.x, pos.y, pos.z);
		modelStack.Scale(0.1, 0.1, 0.1);
		RenderMesh(meshList[BIRCHTREE], true);
		modelStack.PopMatrix();
	}

	modelStack.PushMatrix();
	{

		// spacing chosen to match the previous manual placement (50 units)
		const float start = -170.f;
		const float end = -45.f;
		const float step = 25.f;
		const float cullRadius = 300.f; // tweak: tiles beyond this distance from the camera are skipped
		const float cullRadius2 = cullRadius * cullRadius;

		glm::vec3 camPos = viewStack.Top() * glm::vec4(0, 0, 0, 1);

		// Set material once for the TREE mesh
		meshList[BIRCHTREE]->material.kAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
		meshList[BIRCHTREE]->material.kDiffuse = glm::vec3(0.8f, 0.9f, 0.7f);  // leaf green tint
		meshList[BIRCHTREE]->material.kSpecular = glm::vec3(0.3f, 0.3f, 0.3f);  // subtle highlights
		meshList[BIRCHTREE]->material.kShininess = 15.0f;

		for (float x = start; x <= end; x += step)
		{
			for (float z = start; z <= end; z += step)
			{
				// quick distance cull (squared)
				glm::vec3 center(x, 0.f, z);
				float dist2 = glm::dot(center - camPos, center - camPos);
				if (dist2 > cullRadius2) continue;

				modelStack.PushMatrix();
				modelStack.Translate(x, 0.f, z);
				modelStack.Scale(0.1, 0.1, 0.1);
				RenderMesh(meshList[BIRCHTREE], true);
				modelStack.PopMatrix();
			}
		}
	}
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

	//RenderMesh(meshList[PAUSEMENU], false);
	//RenderMesh(meshList[ENTERBUMPERCARGAMEPROMPT], false);

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
			glm::mat4(0.3f),
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

		/*
		viewStack.LoadIdentity();
		viewStack.LookAt(
			cam.position.x, cam.position.y, cam.position.z,
			cam.target.x, cam.target.y, cam.target.z,
			cam.up.x, cam.up.y, cam.up.z
		);*/

		glm::vec3 renderPos = cam.position;
		glm::vec3 renderTarget = cam.target;

		// tweak these to taste
		const float renderBackDistance = 25.f;  // move camera backward along its forward vector
		const float renderUpOffset = 1.5f;      // lift camera up in world Y

		// If this camera belongs to a player who is in their car, offset the render camera.
		// This does NOT modify cam.position/cam.target (physics/state remain the same).
		if ((&cam == &camera1 && player1InCar) || (&cam == &camera2 && player2InCar))
		{
			// world forward from camera (direction camera is currently facing)
			glm::vec3 forward = glm::normalize(cam.target - cam.position);
			// move the render position backwards and a bit up
			renderPos = cam.position - forward * renderBackDistance + glm::vec3(0.0f, renderUpOffset, 0.0f);
			// keep renderTarget == cam.target so the camera looks slightly down at the same world point
		}

		viewStack.LoadIdentity();
		viewStack.LookAt(
			renderPos.x, renderPos.y, renderPos.z,
			renderTarget.x, renderTarget.y, renderTarget.z,
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
			const float cullRadius = 300.f; // tweak: tiles beyond this distance from the camera are skipped
			const float cullRadius2 = cullRadius * cullRadius;

			glm::vec3 camPos = renderPos; // renderPos is the camera used for viewStack

			// Set material once for the grass mesh
			meshList[SOLIDGREENGRASS]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);

			for (float x = start; x <= end; x += step)
			{
				for (float z = start; z <= end; z += step)
				{
					// quick distance cull (squared)
					glm::vec3 center(x, 0.f, z);
					float dist2 = glm::dot(center - camPos, center - camPos);
					if (dist2 > cullRadius2) continue;

					modelStack.PushMatrix();
					modelStack.Translate(x, 0.f, z);
					modelStack.Scale(5.f, 1.f, 5.f);
					// keep original rotations so the tile faces the same way as before
					modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
					modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
					RenderMesh(meshList[SOLIDGREENGRASS], true);
					modelStack.PopMatrix();
				}
			}
		}
		modelStack.PopMatrix();

		RenderPathway();
		RenderGamePathways();
		RenderBirchTrees();

		/*
		========================================
		BASKETBALL COURT AREA ENVIRONMENT
		========================================
		*/
		{
			modelStack.PushMatrix();
			modelStack.Translate(113, 0.3, 115);
			modelStack.Scale(5, 5, 5);
			modelStack.Rotate(180, 0, 1, 0);
			meshList[BASKETBALLCOURT]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[BASKETBALLCOURT]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[BASKETBALLCOURT]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[BASKETBALLCOURT]->material.kShininess = 5.0f;
			RenderMesh(meshList[BASKETBALLCOURT], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(135, 0, 90);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(180, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(135, 0, 100);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(180, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(135, 0, 110);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(180, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(135, 0, 120);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(180, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(135, 0, 130);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(180, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(90, 0, 70);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(0, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(90, 0, 80);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(0, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(90, 0, 90);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(0, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(90, 0, 100);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(0, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(90, 0, 110);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(0, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			// back buildings

			modelStack.PushMatrix();
			modelStack.Translate(80, 0, 130);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(90, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(90, 0, 130);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(90, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(100, 0, 130);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(90, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(110, 0, 130);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(90, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(120, 0, 130);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(90, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();
		}

		/*
		========================================
		FOREST
		========================================
		*/
		{

			modelStack.PushMatrix();
			modelStack.Translate(300, -2, 0.f);
			modelStack.Scale(20, 20, 20);
			meshList[FOREST]->material.kAmbient = glm::vec3(0, 0, 0);
			RenderMesh(meshList[FOREST], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(300, -2, 150);
			modelStack.Scale(20, 20, 20);
			meshList[FOREST]->material.kAmbient = glm::vec3(0, 0, 0);
			RenderMesh(meshList[FOREST], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(300, -2, -150);
			modelStack.Scale(20, 20, 20);
			meshList[FOREST]->material.kAmbient = glm::vec3(0, 0, 0);
			RenderMesh(meshList[FOREST], false);
			modelStack.PopMatrix();

		}

		{
			{
				// front of fence
				modelStack.PushMatrix();
				modelStack.Translate(76, 0, -42);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(62, 0, -42);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(48, 0, -42);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(20, 0, -42);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(6, 0, -42);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(-8, 0, -42);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();
			}

			{
				// left of fence
				modelStack.PushMatrix();
				modelStack.Translate(-22, 0, -42);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();
					
				modelStack.PushMatrix();
				modelStack.Translate(-22, 0, -56);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(-22, 0, -70);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();	

				modelStack.PushMatrix();
				modelStack.Translate(-22, 0, -84);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(-22, 0, -98);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();
			}

			{
				// back of fence
				modelStack.PushMatrix();
				modelStack.Translate(76, 0, -112);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(62, 0, -112);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(48, 0, -112);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(34, 0, -112);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(20, 0, -112);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(6, 0, -112);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(-8, 0, -112);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();
			}

			{
				// right of fence
				modelStack.PushMatrix();
				modelStack.Translate(76, 0, -42);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(76, 0, -56);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(76, 0, -70);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(76, 0, -84);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(76, 0, -98);
				modelStack.Scale(0.02, 0.02, 0.02);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
				meshList[CARTOONFENCE]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[CARTOONFENCE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[CARTOONFENCE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[CARTOONFENCE]->material.kShininess = 5.0f;
				RenderMesh(meshList[CARTOONFENCE], true);
				modelStack.PopMatrix();
			}

			{
				// BUMPER CAR TENT
				modelStack.PushMatrix();
				modelStack.Translate(26.5, 0, -116);
				modelStack.Scale(51, 35, 70);
				modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
				modelStack.Rotate(180.f, 0, 1, 0);
				meshList[BUMPERCARTENT]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[BUMPERCARTENT]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[BUMPERCARTENT]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[BUMPERCARTENT]->material.kShininess = 5.0f;
				RenderMesh(meshList[BUMPERCARTENT], true);
				modelStack.PopMatrix();
			}
			
		}

		{

			modelStack.PushMatrix();
			modelStack.Translate(-30, 0.f, 66);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(100, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(51, 0.f, 48);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(100, 0.f, 1.f, 0.f);
			meshList[LOWPOLYBUILDING]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[LOWPOLYBUILDING]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[LOWPOLYBUILDING]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[LOWPOLYBUILDING]->material.kShininess = 5.0f;
			RenderMesh(meshList[LOWPOLYBUILDING], true);
			modelStack.PopMatrix();
		}

		modelStack.PushMatrix();
		modelStack.Translate(0.f, 0.f, -25.f);
		modelStack.Scale(1, 1, 1);
		meshList[GEO_COW]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
		meshList[GEO_COW]->material.kDiffuse = glm::vec3(1, 1, 1);
		meshList[GEO_COW]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_COW]->material.kShininess = 5.0f;
		RenderMesh(meshList[GEO_COW], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(-130, 0, 10);
		modelStack.Scale(10, 10, 10);
		meshList[ALVINTENT]->material.kAmbient = glm::vec3(0.1, 0.1, 0.1);
		meshList[ALVINTENT]->material.kDiffuse = glm::vec3(0.8, 0, 0);
		meshList[ALVINTENT]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[ALVINTENT]->material.kShininess = 5.0f;
		RenderMesh(meshList[ALVINTENT], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(110, 0, -5);
		modelStack.Scale(1, 1, 1);
		modelStack.Rotate(310, 0, 1, 0);
		meshList[CARNIVALTENT]->material.kAmbient = glm::vec3(0.1, 0.1, 0.1);
		meshList[CARNIVALTENT]->material.kDiffuse = glm::vec3(0.8, 0, 0);
		meshList[CARNIVALTENT]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[CARNIVALTENT]->material.kShininess = 5.0f;
		RenderMesh(meshList[CARNIVALTENT], true);
		modelStack.PopMatrix();

		
		modelStack.PushMatrix();
		modelStack.Translate(110, 3, 15);
		modelStack.Scale(0.025, 0.025, 0.025);
		modelStack.Rotate(270, 0, 1, 0);
		meshList[CONCERTSTAGE]->material.kAmbient = glm::vec3(0.1, 0.1, 0.1);
		meshList[CONCERTSTAGE]->material.kDiffuse = glm::vec3(0.6, 0.6, 0.6);
		meshList[CONCERTSTAGE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[CONCERTSTAGE]->material.kShininess = 5.0f;
		RenderMesh(meshList[CONCERTSTAGE], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(16, 0, 150);
		modelStack.Scale(1, 1, 1);
		modelStack.Rotate(0, 0, 1, 0);
		meshList[GEO_TENT]->material.kAmbient = glm::vec3(0.1, 0.1, 0.1);
		meshList[GEO_TENT]->material.kDiffuse = glm::vec3(0.8, 0, 0);
		meshList[GEO_TENT]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		meshList[GEO_TENT]->material.kShininess = 5.0f;
		RenderMesh(meshList[GEO_TENT], true);
		modelStack.PopMatrix();

		{
			// CLOWN NPC
			modelStack.PushMatrix();
			modelStack.Translate(-1, 0.3, 54);
			modelStack.Scale(3, 3, 3);
			modelStack.Rotate(200, 0, 1, 0);
			modelStack.Rotate(90, 1, 0, 0);
			meshList[CLOWN]->material.kAmbient = glm::vec3(0.1, 0.1, 0.1);
			meshList[CLOWN]->material.kDiffuse = glm::vec3(0.8, 0, 0);
			meshList[CLOWN]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
			meshList[CLOWN]->material.kShininess = 5.0f;
			RenderMesh(meshList[CLOWN], false);
			modelStack.PopMatrix();
		}

		{
			modelStack.PushMatrix();

			// Apply translation
			modelStack.Translate(objectPos.x, objectPos.y, objectPos.z);

			modelStack.Scale(0.1, 0.1, 0.1);
			RenderMesh(meshList[CLOWN], false);
			modelStack.PopMatrix();

		}

		if (!player1InCar)
		{
			modelStack.PushMatrix();
			modelStack.Translate(-11, 0, -90);
			modelStack.Scale(3, 3, 3);
			meshList[BUMPERCAR]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[BUMPERCAR]->material.kDiffuse = glm::vec3(1, 1, 1);
			meshList[BUMPERCAR]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[BUMPERCAR]->material.kShininess = 5.0f;
			RenderMesh(meshList[BUMPERCAR], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-11, 6, -85);
			modelStack.Scale(1, 1, 1);
			modelStack.Rotate(90, 0, 1, 0);
			RenderText(meshList[GEO_TEXT], "Player 1's car!", glm::vec3(1, 0, 0));
			modelStack.PopMatrix();

			// ----- Render Player 1 Model -----
			if (!isCamera1)  // If current view is NOT camera1
			{
				modelStack.PushMatrix();
				modelStack.Translate(camera1.position.x, 0.5, camera1.position.z);
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

				meshList[GEO_COW]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[GEO_COW]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[GEO_COW]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[GEO_COW]->material.kShininess = 5.0f;
				RenderMesh(meshList[GEO_COW], true);
				modelStack.PopMatrix();
			}
		}

		if (!player2InCar)
		{
			modelStack.PushMatrix();
			modelStack.Translate(-11, 0, -60);
			modelStack.Scale(3, 3, 3);
			meshList[BUMPERCAR]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[BUMPERCAR]->material.kDiffuse = glm::vec3(1, 1, 1);
			meshList[BUMPERCAR]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[BUMPERCAR]->material.kShininess = 5.f;
			RenderMesh(meshList[BUMPERCAR], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-11, 6, -55);
			modelStack.Scale(1, 1, 1);
			modelStack.Rotate(90, 0, 1, 0);
			RenderText(meshList[GEO_TEXT], "Player 2's car!", glm::vec3(1, 0, 0));
			modelStack.PopMatrix();

			// ----- Render Player 2 Model -----
			if (isCamera1)  // If current view IS camera1
			{
				modelStack.PushMatrix();
				modelStack.Translate(camera2.position.x, 0.5, camera2.position.z);
				modelStack.Scale(2, 2, 2);
				modelStack.Rotate(90.f, 0.f, 1.f, 0.f);

				// Make the model face the direction camera2 is facing:
				{
					glm::vec3 pForward = glm::normalize(camera2.target - camera2.position);
					// Yaw in degrees from X axis
					float yaw = glm::degrees(atan2(pForward.z, pForward.x));
					// Rotate the model around world Y so it faces the same horizontal direction
					modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
				}

				meshList[GEO_COW]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
				meshList[GEO_COW]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[GEO_COW]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
				meshList[GEO_COW]->material.kShininess = 5.0f;
				RenderMesh(meshList[GEO_COW], true);
				modelStack.PopMatrix();
			}
		}

		if (player1InCar)
		{
			// Render Player 1 Model so that both other cameras AND the player's own camera can see it.
			// When the current camera is camera1 (isCamera1 == true) we offset the model slightly behind
			// the camera so the player can still see their own car (simple 3rd-person view).
			modelStack.PushMatrix();

			// Compute draw position: either at the car position (other cameras) or slightly behind for own-camera view
			glm::vec3 drawPos1 = camera1.position;
			if (isCamera1)
			{
				glm::vec3 forward1 = glm::normalize(camera1.target - camera1.position);
				const float behindDistance = 8.0f; // tweak to taste
				drawPos1 += -forward1 * behindDistance;
			}

			modelStack.PushMatrix();
			modelStack.Translate(drawPos1.x, 0.5f, drawPos1.z);
			modelStack.Scale(3.f, 3.f, 3.f);

			// Make the model face the movement direction (based on forward vector)
			{
				// Recompute forward here (same as in driving code)
				float cosYaw = cos(carYaw);
				float sinYaw = sin(carYaw);
				glm::vec3 forward = -glm::vec3(sinYaw, 0.0f, cosYaw);  // Inverted, matches driving
				glm::vec3 pForward = glm::normalize(forward);  // Ensure normalized
				float yaw = glm::degrees(atan2(pForward.z, pForward.x));
				modelStack.Rotate(yaw, 0.f, -1.f, 0.f);  // Use -Y axis to match original
			}
			meshList[BUMPERCAR]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[BUMPERCAR]->material.kDiffuse = glm::vec3(1, 1, 1);
			meshList[BUMPERCAR]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[BUMPERCAR]->material.kShininess = 5.f;
			RenderMesh(meshList[BUMPERCAR], true);
			modelStack.PopMatrix();

			// Render the cow model on/near the car as before (also offset for own-camera)
			modelStack.PushMatrix();
			glm::vec3 cowPos1 = camera1.position;
			if (isCamera1)
			{
				glm::vec3 forward1 = glm::normalize(camera1.target - camera1.position);
				const float behindDistance = 8.0f;
				cowPos1 += -forward1 * behindDistance;
			}
			modelStack.Translate(cowPos1.x, 1.f, cowPos1.z);
			modelStack.Scale(2.f, 2.f, 2.f);
			modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
			{
				glm::vec3 pForward = glm::normalize(camera1.target - camera1.position);
				float yaw = glm::degrees(atan2(pForward.z, pForward.x));
				modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
			}
			meshList[GEO_COW]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[GEO_COW]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[GEO_COW]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[GEO_COW]->material.kShininess = 5.0f;
			RenderMesh(meshList[GEO_COW], true);
			modelStack.PopMatrix();
		}
		if (player2InCar)
		{
			// Render Player 2 Model so that both other cameras AND the player's own camera can see it.
			// When the current camera is camera2 (isCamera2 == true) we offset the model slightly behind
			// the camera so the player can still see their own car.
			modelStack.PushMatrix();

			glm::vec3 drawPos2 = camera2.position;
			if (isCamera2)
			{
				glm::vec3 forward2 = glm::normalize(camera2.target - camera2.position);
				const float behindDistance = 8.0f; // same adjustable distance
				drawPos2 += -forward2 * behindDistance;
			}

			modelStack.PushMatrix();
			modelStack.Translate(drawPos2.x, 0.5f, drawPos2.z);
			modelStack.Scale(3.f, 3.f, 3.f);

			// Make the model face the movement direction (based on forward vector)
			{
				// Recompute forward here (same as in driving code)
				float cosYaw = cos(carYaw2);
				float sinYaw = sin(carYaw2);
				glm::vec3 forward = -glm::vec3(sinYaw, 0.0f, cosYaw);  // Inverted, matches driving
				glm::vec3 pForward = glm::normalize(forward);  // Ensure normalized
				float yaw = glm::degrees(atan2(pForward.z, pForward.x));
				modelStack.Rotate(yaw, 0.f, -1.f, 0.f);  // Use -Y axis to match original
			}
			meshList[BUMPERCAR]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[BUMPERCAR]->material.kDiffuse = glm::vec3(1, 1, 1);
			meshList[BUMPERCAR]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[BUMPERCAR]->material.kShininess = 5.f;
			RenderMesh(meshList[BUMPERCAR], true);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			glm::vec3 cowPos2 = camera2.position;
			if (isCamera2)
			{
				glm::vec3 forward2 = glm::normalize(camera2.target - camera2.position);
				const float behindDistance = 8.0f;
				cowPos2 += -forward2 * behindDistance;
			}
			modelStack.Translate(cowPos2.x, 1, cowPos2.z);
			modelStack.Scale(2.f, 2.f, 2.f);
			modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
			{
				glm::vec3 pForward = glm::normalize(camera2.target - camera2.position);
				float yaw = glm::degrees(atan2(pForward.z, pForward.x));
				modelStack.Rotate(yaw, 0.f, -1.f, 0.f);
			}
			meshList[GEO_COW]->material.kAmbient = glm::vec3(1.f, 1.f, 1.f);
			meshList[GEO_COW]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[GEO_COW]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[GEO_COW]->material.kShininess = 5.0f;
			RenderMesh(meshList[GEO_COW], true);
			modelStack.PopMatrix();
		}
	}
}


void Scene01::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (pausemenu)
	{
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, 1600, 900);
		RenderMeshOnScreen(meshList[MAINPAUSE], 800, 450, 1600, 900);
		glEnable(GL_DEPTH_TEST);
		return;
	}

	if (BumperCarGameEntered)
	{
		int width, height;
		glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

		// LEFT SCREEN
		glViewport(0, 0, width / 2, height);
		glClear(GL_DEPTH_BUFFER_BIT);

		projectionStack.LoadMatrix(glm::perspective(
			glm::radians(45.f),
			(float)(width / 2) / (float)height,
			0.1f,
			1000.f));

		RenderSceneFromCamera(camera1);

		// RIGHT SCREEN
		glViewport(width / 2, 0, width / 2, height);
		glClear(GL_DEPTH_BUFFER_BIT);

		projectionStack.LoadMatrix(glm::perspective(
			glm::radians(45.f),
			(float)(width / 2) / (float)height,
			0.1f,
			1000.f));

		RenderSceneFromCamera(camera2);

		// Exit Prompt
		if (ExitBumperCarGamePrompt)
		{
			glDisable(GL_DEPTH_TEST);
			glViewport(0, 0, 1600, 900);
			RenderMeshOnScreen(meshList[EXITBUMPERCARGAMEPROMPT], 800, 450, 1600, 900);
			glEnable(GL_DEPTH_TEST);
		}
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

		glViewport(0, 0, width, height);

		projectionStack.LoadMatrix(
			glm::perspective(
				glm::radians(45.f),
				(float)width / (float)height,
				0.1f,
				1000.f
			)
		);

		RenderSceneFromCamera(camera1);

		if (EnterBumperCarGamePrompt)
		{
			glDisable(GL_DEPTH_TEST);
			RenderMeshOnScreen(meshList[ENTERBUMPERCARGAMEPROMPT], 800, 450, 1600, 900);
			glEnable(GL_DEPTH_TEST);
		}

		if (EnterBasketballGamePrompt)
		{
			glDisable(GL_DEPTH_TEST);
			RenderMeshOnScreen(meshList[ENTERBASKETBALLPROMPT], 800, 450, 1600, 900);
			glEnable(GL_DEPTH_TEST);
		}

		if (EnterBallBouncerGamePrompt)
		{
			glDisable(GL_DEPTH_TEST);
			RenderMeshOnScreen(meshList[ENTERBALLBOUNCERPROMPT], 800, 450, 1600, 900);
			glEnable(GL_DEPTH_TEST);
		}

		if (EnterDuckShootingGamePrompt)
		{
			glDisable(GL_DEPTH_TEST);
			RenderMeshOnScreen(meshList[ENTERDUCKSHOOTINGPROMPT], 800, 450, 1600, 900);
			glEnable(GL_DEPTH_TEST);
		}
	}
	std::string temp("FPS:" + std::to_string(fps));
	RenderTextOnScreen(meshList[GEO_TEXT], temp.substr(0, 9), glm::vec3(1, 1, 1), 25, 5, 560);

}

void Scene01::RenderMesh(Mesh* mesh, bool enableLight)
{
	// Compute matrices
	glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));
	glm::mat4 modelView = viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, glm::value_ptr(modelView));

	// Always explicitly set light enabled uniform for correct per-draw state
	if (enableLight) {
		glUniform1i(m_parameters[U_LIGHTENABLED], 1);

		glm::mat4 modelView_inverse_transpose = glm::inverseTranspose(modelView);
		glUniformMatrix4fv(m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE], 1, GL_FALSE, glm::value_ptr(modelView_inverse_transpose));

		// material must be supplied per-mesh
		glUniform3fv(m_parameters[U_MATERIAL_AMBIENT], 1, &mesh->material.kAmbient.r);
		glUniform3fv(m_parameters[U_MATERIAL_DIFFUSE], 1, &mesh->material.kDiffuse.r);
		glUniform3fv(m_parameters[U_MATERIAL_SPECULAR], 1, &mesh->material.kSpecular.r);
		glUniform1f(m_parameters[U_MATERIAL_SHININESS], mesh->material.kShininess);
	}
	else {
		glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	}

	// Explicitly bind texture every draw to avoid cross-viewport caching issues
	if (mesh->textureID > 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	}
	else {
		// ensure no texture is bound
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 0);
	}

	mesh->Render();
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

// ------- Modified input handlers (free-camera collision checks) -------

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
		// Use aggregated movement and test fence collisions BEFORE applying.
		float movement = moveSpeed * static_cast<float>(dt);
		glm::vec3 moveVec(0.f);

		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W)) moveVec += forward * movement;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S)) moveVec -= forward * movement;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A)) moveVec -= right * movement;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D)) moveVec += right * movement;

		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT))
			moveVec *= 3.f;

		if (glm::length(moveVec) > 0.0001f)
		{
			glm::vec3 candidatePos = cam.position + moveVec;
			if (!IsInsideFence(candidatePos))
			{
				cam.position = candidatePos;
				cam.target += moveVec;
			}
			// else: movement blocked by fence
		}

		if (!player1InCar && !player2InCar && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_SPACE))
		{
			// Only start a jump if grounded / not already jumping
			const float groundY = 3.3f;
			if (!isJumping1 && camera1.position.y <= groundY + 0.05f)
			{
				isJumping1 = true;
				cameraVerticalVel1 = jumpSpeed;
			}
		}

		// Clamp camera height to adjusted limits
		if (cam.position.y < 3.3f) {
			cam.position.y = 3.3f;
			if (cam.target.y < 3.3f)
				cam.target.y = 3.3f;
		}
		// Only enforce the upper limit when not jumping
		if (!isJumping1 && cam.position.y > 3.5f) {
			cam.position.y = 3.5f;
			if (cam.target.y > 3.5f)
				cam.target.y = 3.5f;
		}
	}

	if (player1InCar)
	{
		// Constants for realistic driving (tune as needed)
		const float turnRate = 2.5f;          // Radians per second for turning
		const float maxSpeed = 40.0f;         // Max speed in m/s
		const float dragCoefficient = 0.92f;  // Drag factor per second
		const float brakeAcceleration = 18.0f; // Deceleration when braking

		// Compute car's local forward and right vectors based on yaw
		float cosYaw = cos(carYaw);
		float sinYaw = sin(carYaw);
		glm::vec3 forward = -glm::vec3(sinYaw, 0.0f, cosYaw);    // Forward direction (XZ plane)
		glm::vec3 right(cosYaw, 0.0f, -sinYaw);     // Right direction (XZ plane)

		// Handle acceleration (forward/backward only)
		float accelMagnitude = 0.0f;
		bool isBraking = false;
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W)) {
			accelMagnitude += driveAcceleration;
		}
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S)) {
			accelMagnitude -= brakeAcceleration;
			isBraking = true;
		}

		// Apply boost if shift is held (only for acceleration, not braking)
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT) && !isBraking) {
			accelMagnitude *= 1.5f;
		}

		// Apply acceleration to velocity (only along XZ)
		glm::vec3 accelDir = forward * accelMagnitude;
		*pVel += accelDir * static_cast<float>(dt);

		// Handle turning (A/D keys rotate the car's yaw) (swapped to see if things work)
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D)) {
			carYaw -= turnRate * static_cast<float>(dt);
		}
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A)) {
			carYaw += turnRate * static_cast<float>(dt);
		}

		// Apply drag/friction
		glm::vec3 velXZ = glm::vec3((*pVel).x, 0.0f, (*pVel).z);
		float speed = glm::length(velXZ);
		if (speed > 0.001f) {
			velXZ *= pow(dragCoefficient, static_cast<float>(dt));
			(*pVel).x = velXZ.x;
			(*pVel).z = velXZ.z;
		}

		// Clamp speed to maximum
		speed = glm::length(velXZ);
		if (speed > maxSpeed) {
			velXZ = glm::normalize(velXZ) * maxSpeed;
			(*pVel).x = velXZ.x;
			(*pVel).z = velXZ.z;
		}

		// Clamp camera height (unchanged)
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

		// Removed: Camera target locking to car's forward (allows free-look)

		// Ensure FPCamera internal vectors are recalculated
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
		const glm::vec3 minPos(-21, -1, -100);
		const glm::vec3 maxPos(-1, 5, -80);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		if (isInsideBox(cam.position, minPos, maxPos))
		{
			enterCar1Check = true;
			player1InCar = true;
		}
	}

	{
		// ENTER BUMPER CAR PROMPT
		const glm::vec3 minPos(-19, -1, -108);
		const glm::vec3 maxPos(73, 6, -43);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		if (isInsideBox(camera1.position, minPos, maxPos))
		{
			EnterBumperCarGamePrompt = true;

			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_ENTER))
			{
				EnterBumperCarGamePrompt = false;
				BumperCarGameEntered = true;
			}
			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F1))
			{
				EnterBumperCarGamePrompt = false;
				BumperCarGameEntered = false;
			}
		}

		if (BumperCarGameEntered)
		{
			if (!isInsideBox(camera1.position, minPos, maxPos))
			{
				ExitBumperCarGamePrompt = true;

				if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_ENTER))
				{
					BumperCarGameEntered = false;
					ExitBumperCarGamePrompt = false;
				}
				if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F1))
				{
					BumperCarGameEntered = true;
					ExitBumperCarGamePrompt = false;
				}
			}
		}
	}
}

void Scene01::HandleKeyPress2(FPCamera& cam, double dt)
{
	if (BumperCarGameEntered)
	{
		// common toggles (culling, polygon mode, background, lights)
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

			const float rotationSpeed = 125.0f; // Adjusted for real dt
			float turnAmount = (rotationSpeed * dt);

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
		}

		glm::vec3* pVel = (&cam == &camera1) ? &cameraVelocity1 : &cameraVelocity2;

		// --- MOVEMENT based on the up-to-date orientation ---
		glm::vec3 forward = glm::normalize(cam.target - cam.position);
		glm::vec3 right = glm::normalize(glm::cross(forward, cam.up));

		if (!player2InCar)
		{
			float movement = moveSpeed * static_cast<float>(dt);

			// Aggregate movement so we can test collision
			glm::vec3 moveVec(0.0f);
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP))    moveVec += forward * movement;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_DOWN))  moveVec -= forward * movement;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT))  moveVec -= right * movement;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT)) moveVec += right * movement;

			if (glm::length(moveVec) > 0.0001f)
			{
				glm::vec3 candidatePos = cam.position + moveVec;
				if (!IsInsideFence(candidatePos))
				{
					cam.position = candidatePos;
				}
				// else blocked
			}

			if (!player2InCar && !player1InCar && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_RIGHT_CONTROL))
			{
				const float groundY = 3.3f;
				if (!isJumping2 && camera2.position.y <= groundY + 0.05f)
				{
					isJumping2 = true;
					cameraVerticalVel2 = jumpSpeed;
				}
			}

			cam.position.y = (glm::max)(cam.position.y, 3.3f);      // always enforce lower bound
			if (!isJumping2)
			{
				cam.position.y = (glm::min)(cam.position.y, 3.5f);
			}

			// 3. RE-CALCULATE target based on current position + stored angles
			float az = glm::radians(cam.azimuth);
			float alt = glm::radians(cam.altitude);

			glm::vec3 dir;
			dir.x = cosf(alt) * cosf(az);
			dir.y = sinf(alt);
			dir.z = cosf(alt) * sinf(az);

			cam.target = cam.position + dir;

			// 4. Update internal camera vectors
			cam.Init(cam.position, cam.target, glm::vec3(0.0f, 1.0f, 0.0f));

			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_C))
			{
				player2InCar = true;
			}
		}
		
		if (player2InCar)
		{
			// Constants for realistic driving
			const float turnRate = 2.5f;
			const float maxSpeed = 40.0f;
			const float dragCoefficient = 0.92f;
			const float brakeAcceleration = 18.0f;

			// Compute car's local forward and right vectors based on yaw
			float cosYaw = cos(carYaw2);
			float sinYaw = sin(carYaw2);
			glm::vec3 forward = -glm::vec3(sinYaw, 0.0f, cosYaw);
			glm::vec3 right(cosYaw, 0.0f, -sinYaw);

			// Handle acceleration
			float accelMagnitude = 0.0f;
			bool isBraking = false;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP)) {
				accelMagnitude += driveAcceleration;
			}
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_DOWN)) {
				accelMagnitude -= brakeAcceleration;
				isBraking = true;
			}

			// Apply boost
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT_SHIFT) && !isBraking) {
				accelMagnitude *= 1.5f;
			}

			// Apply acceleration to velocity
			glm::vec3 accelDir = forward * accelMagnitude;
			*pVel += accelDir * static_cast<float>(dt);

			// Handle turning (LEFT/RIGHT keys rotate the car's yaw) (swapped to see things)
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT)) {
				carYaw2 -= turnRate * static_cast<float>(dt);
			}
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT)) {
				carYaw2 += turnRate * static_cast<float>(dt);
			}

			// Apply drag/friction
			glm::vec3 velXZ = glm::vec3((*pVel).x, 0.0f, (*pVel).z);
			float speed = glm::length(velXZ);
			if (speed > 0.001f) {
				velXZ *= pow(dragCoefficient, static_cast<float>(dt));
				(*pVel).x = velXZ.x;
				(*pVel).z = velXZ.z;
			}

			// Clamp speed
			speed = glm::length(velXZ);
			if (speed > maxSpeed) {
				velXZ = glm::normalize(velXZ) * maxSpeed;
				(*pVel).x = velXZ.x;
				(*pVel).z = velXZ.z;
			}

			// Clamp camera height
			cam.position.y = glm::clamp(cam.position.y, 3.3f, 3.5f);

			// --- Camera Free-Look Rotation (independent of car facing) ---
			const float rotationSpeed = 125.0f;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_I)) cam.altitude += rotationSpeed * dt;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_K)) cam.altitude -= rotationSpeed * dt;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_J)) cam.azimuth -= rotationSpeed * dt;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_L)) cam.azimuth += rotationSpeed * dt;

			// Clamp altitude
			cam.altitude = glm::clamp(cam.altitude, -89.0f, 89.0f);
			cam.azimuth = fmod(cam.azimuth + 360.0f, 360.0f);

			// Update camera target based on azimuth/altitude (free-look)
			float azRad = glm::radians(cam.azimuth);
			float altRad = glm::radians(cam.altitude);
			glm::vec3 lookDir;
			lookDir.x = cosf(altRad) * cosf(azRad);
			lookDir.y = sinf(altRad);
			lookDir.z = cosf(altRad) * sinf(azRad);
			cam.target = cam.position + lookDir;

			// Update internal camera vectors
			cam.Init(cam.position, cam.target, glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}
}