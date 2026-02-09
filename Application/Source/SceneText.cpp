#include "SceneText.h"
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

SceneText::SceneText()
{
}

SceneText::~SceneText()
{
}

// AABB overlap implementation adapted to use glm::vec3 and the local CollisionData.
bool SceneText::OverlapAABB2AABB(const glm::vec3& pos1, float w1, float h1,
	const glm::vec3& pos2, float w2, float h2, CollisionData& cd)
{
	// compute axis-aligned mins/maxs in X/Y (z stays zero for 2D overlap)
	glm::vec3 min1 = pos1 - glm::vec3(w1, h1, 0.f);
	glm::vec3 max1 = pos1 + glm::vec3(w1, h1, 0.f);
	glm::vec3 min2 = pos2 - glm::vec3(w2, h2, 0.f);
	glm::vec3 max2 = pos2 + glm::vec3(w2, h2, 0.f);

	// quick rejection test
	if (max1.x < min2.x || min1.x > max2.x || max1.y < min2.y || min1.y > max2.y)
		return false;

	// compute overlap on each axis
	float overlapX = (std::min)(max1.x, max2.x) - (std::max)(min1.x, min2.x);
	float overlapY = (std::min)(max1.y, max2.y) - (std::max)(min1.y, min2.y);

	// choose smallest penetration axis
	if (overlapX < overlapY)
	{
		// normal points from box1 -> box2 along x
		cd.collisionNormal = (pos1.x < pos2.x) ? glm::vec3(-1.f, 0.f, 0.f) : glm::vec3(1.f, 0.f, 0.f);
		cd.penetration = overlapX;
	}
	else
	{
		cd.collisionNormal = (pos1.y < pos2.y) ? glm::vec3(0.f, -1.f, 0.f) : glm::vec3(0.f, 1.f, 0.f);
		cd.penetration = overlapY;
	}

	// approximate contact point (center of overlap region)
	cd.contactPoint.x = ((std::max)(min1.x, min2.x) + (std::min)(max1.x, max2.x)) * 0.5f;
	cd.contactPoint.y = ((std::max)(min1.y, min2.y) + (std::min)(max1.y, max2.y)) * 0.5f;
	cd.contactPoint.z = 0.f;

	return true;
}

void SceneText::Init()
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

	meshList[GEO_ZUL] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_ZUL]->textureID = LoadTGA("Images//zulmobile.tga");

	meshList[GEO_GRASS] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_GRASS]->textureID = LoadTGA("Images//coast_sand_rocks_02 copy.tga");

	//meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	//meshList[GEO_GUI]->textureID = LoadTGA("Images//color.tga");

	// 16 x 16 is the number of columns and rows for the text
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//Georgia.tga");

	meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_GUI]->textureID = LoadTGA("Images//blackblack.tga");

	meshList[GEO_EYEBALL] = MeshBuilder::GenerateOBJMTL("eyeballmtl", "Models//eyeball.obj", "Models//eyeball.mtl");
	meshList[GEO_EYEBALL]->textureID = LoadTGA("Images//Eye_D.tga");

	meshList[GEO_TOWER] = MeshBuilder::GenerateOBJMTL("tower", "Models//wooden watch tower2.obj", "Models//wooden watch tower2.mtl");
	meshList[GEO_TOWER]->textureID = LoadTGA("Images//Wood_Tower_Col.tga");

	meshList[GEO_PINETREE] = MeshBuilder::GenerateOBJMTL("tower", "Models//DeadTree_LoPoly.obj", "Models//DeadTree_LoPoly.mtl");
	meshList[GEO_PINETREE]->textureID = LoadTGA("Images//DeadTree_LoPoly_DeadTree_Diffuse copy.tga");

	//meshList[GEO_PEWPEW] = MeshBuilder::GenerateOBJMTL("tower", "Models//low-poly_geissele_urg-i_14.5.obj", "Models//low-poly_geissele_urg-i_14.5.mtl");

	meshList[GEO_ABANDONEDHOUSE] = MeshBuilder::GenerateOBJMTL("abandonedhse", "Models//abandoned_house.obj", "Models//abandoned_house.mtl");
	meshList[GEO_ABANDONEDHOUSE]->textureID = LoadTGA("Images//abandonedhouseBaseColor.tga");

	meshList[GEO_ABANDONEDHOUSE2] = MeshBuilder::GenerateOBJMTL("abandonedhse", "Models//abandonedwoodhouse.obj", "Models//abandonedwoodhouse.mtl");
	meshList[GEO_ABANDONEDHOUSE2]->textureID = LoadTGA("Images//woodabandonedhouse copy.tga");

	meshList[GEO_SHADOW] = MeshBuilder::GenerateOBJMTL("shadow", "Models//shadowretry.obj", "Models//shadowretry.mtl");

	meshList[GEO_FLASHLIGHT] = MeshBuilder::GenerateOBJMTL("flashlight", "Models//low_poly_flashlight.obj", "Models//low_poly_flashlight.mtl");

	meshList[GEO_NOTE] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_NOTE]->textureID = LoadTGA("Images//note.tga");

	meshList[GEO_OBJECTIVE_TEXT] = MeshBuilder::GenerateQuad("objectivetext", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_OBJECTIVE_TEXT]->textureID = LoadTGA("Images//objectivetexttest.tga");

	meshList[GEO_SPARKLING_STAR] = MeshBuilder::GenerateOBJMTL("sparklingstar", "Models//sparkling_star.obj", "Models//sparkling_star.mtl");

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

	// Fog uniforms (locations)
	m_parameters[U_FOG_ENABLED] = glGetUniformLocation(m_programID, "fogEnabled");
	m_parameters[U_FOG_START] = glGetUniformLocation(m_programID, "fogStart");
	m_parameters[U_FOG_END] = glGetUniformLocation(m_programID, "fogEnd");
	m_parameters[U_FOG_COLOR] = glGetUniformLocation(m_programID, "fogColor");

	// Set default fog values
	glUniform1i(m_parameters[U_FOG_ENABLED], fogEnabled ? 1 : 0);
	glUniform1f(m_parameters[U_FOG_START], fogStart);
	glUniform1f(m_parameters[U_FOG_END], fogEnd);
	glUniform3fv(m_parameters[U_FOG_COLOR], 1, &fogColor.r);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	isPlayerDead = false;

}

void SceneText::HandleMouseInput() {

	if (isPlayerDead) {
		return;  // Skip mouse input when dead
	}

	// Skip mouse input if stillness is active
	if (isShadowSpawned && shadowStillTimer < 1.0f)
	{
		return;
	}

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

// Sweep the same grid used in Render() and resolve camera <-> tree AABB overlaps (XZ-plane).
void SceneText::HandleTreeCollisions()
{
    // base radii (tune these to match your camera/player and tree footprint)
    const float camRadiusBase = 0.35f;   // half-width approximated for the camera/player
    const float treeRadiusBase = 0.6f;   // approximate tree footprint (after model scale)
    const float extraRange = 3.f;       // <-- increase this to enlarge collision range

    const float camRadius = camRadiusBase + extraRange;
    const float treeRadius = treeRadiusBase + extraRange;
    const float combinedRadius = camRadius + treeRadius;
    const float combinedRadiusSq = combinedRadius * combinedRadius;

    // Narrow search to a neighborhood around camera for performance
    const float searchRadius = combinedRadius + kTreeStep * 2.0f; // check a couple of rings
    const float minX = camera.position.x - searchRadius;
    const float maxX = camera.position.x + searchRadius;
    const float minZ = camera.position.z - searchRadius;
    const float maxZ = camera.position.z + searchRadius;

    // Convert world range to grid indices (kTreeStart..kTreeEnd step kTreeStep)
    int ixStart = static_cast<int>(std::floor((minX - kTreeStart) / kTreeStep));
    int ixEnd   = static_cast<int>(std::ceil ((maxX - kTreeStart) / kTreeStep));
    int izStart = static_cast<int>(std::floor((minZ - kTreeStart) / kTreeStep));
    int izEnd   = static_cast<int>(std::ceil ((maxZ - kTreeStart) / kTreeStep));

    // clamp indices to grid bounds
    auto clampIndex = [](int i, float start, float end, float step) -> int {
        const int minIdx = 0;
        const int maxIdx = static_cast<int>(std::floor((end - start) / step));
		return (std::max)(minIdx, (std::min)(i, maxIdx));
    };
    ixStart = clampIndex(ixStart, kTreeStart, kTreeEnd, kTreeStep);
    ixEnd   = clampIndex(ixEnd,   kTreeStart, kTreeEnd, kTreeStep);
    izStart = clampIndex(izStart, kTreeStart, kTreeEnd, kTreeStep);
    izEnd   = clampIndex(izEnd,   kTreeStart, kTreeEnd, kTreeStep);

    for (int ix = ixStart; ix <= ixEnd; ++ix)
    {
        float x = kTreeStart + ix * kTreeStep;
        for (int iz = izStart; iz <= izEnd; ++iz)
        {
            float z = kTreeStart + iz * kTreeStep;
            glm::vec3 treePos = glm::vec3(x, -1.0f, z); // tree placement used in Render()

            // XZ vector from tree to camera
            float dx = camera.position.x - treePos.x;
            float dz = camera.position.z - treePos.z;
            float distSq = dx * dx + dz * dz;

            if (distSq < combinedRadiusSq && distSq > 1e-6f)
            {
                float dist = std::sqrt(distSq);
                float penetration = combinedRadius - dist;

                // push camera away along the XZ direction
                glm::vec3 push;
                push.x = (dx / dist) * penetration;
                push.y = 0.0f;
                push.z = (dz / dist) * penetration;

                camera.position += push;
                camera.target += push;
                camera.Init(camera.position, camera.target, camera.up);

                // Optionally stop after first resolution to avoid stacking responses:
                // return;
            }
            else if (distSq <= 1e-6f)
            {
                // camera exactly at tree center: choose arbitrary push direction
                glm::vec3 push = glm::vec3(combinedRadius, 0.0f, 0.0f);
                camera.position += push;
                camera.target += push;
                camera.Init(camera.position, camera.target, camera.up);
                // return;
            }
        }
    }
}

void SceneText::Update(double dt)
{
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

	HandleTreeCollisions();

	// Prevent camera from going below ground after camera updates
	if (camera.position.y < 3.0f) {
		camera.position.y = 3.0f;
		if (camera.target.y < 3.0f)
			camera.target.y = 3.0f;
		camera.Init(camera.position, camera.target, camera.up);
	}

	if (startOfGame)
	{
		if (StarttextTimer > 0) {
			StarttextTimer -= static_cast<float>(dt); // Subtract the time passed since the last frame
		}
	}

	light[0].position = glm::vec3(camera.position.x, camera.position.y, camera.position.z);

	if (flashlightOn)
	{
		if (flashlighttextTimer > 0) {
			flashlighttextTimer -= static_cast<float>(dt); // Subtract the time passed since the last frame
		}
	}

	// Shadow rise cutscene
	{
		// Define the minPos and maxPos variables in the correct scope
		const glm::vec3 minPos(65.0f, 1.0f, 25.0f);
		const glm::vec3 maxPos(85.0f, 4.0f, 45.0f);

		// Define the cutsceneActivation lambda in the same scope
		auto cutsceneActivation = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		if (!isShadowSpawned && cutsceneActivation(camera.position, minPos, maxPos))
		{
			isShadowSpawned = true;
		}

		if (isShadowSpawned)
		{
			if (ChasetextTimer > 0) {
				ChasetextTimer -= static_cast<float>(dt); // Subtract the time passed since the last frame
			}

			// Start stillness timer on activation
			if (!shadowStillActive)
			{
				shadowStillActive = true;
				shadowStillTimer = 0.0f;
			}
			shadowStillTimer += static_cast<float>(dt);

			if (shadowY < shadowTargetY)
			{
				shadowY += (float)(shadowRiseSpeed * dt);
				// Ensure we don't overshoot the target
				if (shadowY > shadowTargetY) shadowY = shadowTargetY;
			}

			// Set moveSpeed: 0 during stillness, 15.f after
			moveSpeed = (shadowStillTimer < 1.0f) ? 0.0f : 15.0f;

			// Start the follow delay timer when shadow is spawned
			if (!shadowFollowStarted)
			{
				shadowFollowStarted = true;
				shadowFollowTimer = 0.0f;
				// Initialize shadow position to the fixed starting point
				shadowCurrentPos = glm::vec3(90.0f, shadowY, 37.0f);
			}

			shadowFollowTimer += static_cast<float>(dt);

			{
				// Calculate 3D distance between camera and shadow position
				glm::vec3 shadowPos(shadowCurrentPos.x, shadowY, shadowCurrentPos.z);
				float dist = glm::distance(camera.position, shadowPos);

				// Debug: Log distance every frame (remove or comment out after testing)
				std::cout << "Distance to shadow: " << dist << std::endl;

				// Trigger death if within 0.5 units (adjust threshold as needed for balance)
				if (dist < 3.f) {
					isPlayerDead = true;
					// Debug: Only log when actually touching
					std::cout << "touched shadow" << std::endl;
				}
			}
		}
	}

	HandleMouseInput();

}

void SceneText::RenderSkybox()
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

void SceneText::RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey)
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

void SceneText::RenderText(Mesh* mesh, std::string text, glm::vec3 color)
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


void SceneText::RenderTextOnScreen(Mesh* mesh, std::string
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

void SceneText::RenderFlashlight()
{
	glm::vec3 forward = glm::normalize(camera.target - camera.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	modelStack.PushMatrix();

	if (!flashlightOn)
	{
		// World-space flashlight (original behaviour)
		modelStack.Translate(-10.f, 1.f, 38.f);
	}
	else
	{
		const float forwardOffset = 0.6f;
		const float rightOffset = 0.25f;
		const float upOffset = -0.25f;

		glm::vec3 holdOffset = forward * forwardOffset + right * rightOffset + up * upOffset;
		glm::vec3 worldPos = camera.position + holdOffset;

		// Render the handheld flashlight mesh oriented to the camera
		float yaw = glm::degrees(atan2f(forward.x, forward.z));
		float pitch = glm::degrees(asinf(forward.y));

		modelStack.PushMatrix();
		modelStack.Translate(worldPos.x, worldPos.y, worldPos.z);
		modelStack.Rotate(yaw, 0.f, 1.f, 0.f);
		modelStack.Rotate(-pitch, 1.f, 0.f, 0.f);
		modelStack.Rotate(180.f, 1.f, 0.f, 0.f);
		modelStack.Scale(0.2f, 0.2f, 0.2f); // adjust as needed
		meshList[GEO_FLASHLIGHT]->material.kAmbient = glm::vec3(0.3f);
		meshList[GEO_FLASHLIGHT]->material.kDiffuse = glm::vec3(0.6f);
		meshList[GEO_FLASHLIGHT]->material.kSpecular = glm::vec3(0.8f);
		meshList[GEO_FLASHLIGHT]->material.kShininess = 5.0f;
		RenderMesh(meshList[GEO_FLASHLIGHT], true);
		modelStack.PopMatrix();
	}

	modelStack.Scale(2.f, 2.f, 2.f);

	meshList[GEO_FLASHLIGHT]->material.kAmbient = glm::vec3(0.3f);
	meshList[GEO_FLASHLIGHT]->material.kDiffuse = glm::vec3(0.6f);
	meshList[GEO_FLASHLIGHT]->material.kSpecular = glm::vec3(0.8f);
	meshList[GEO_FLASHLIGHT]->material.kShininess = 5.0f;

	RenderMesh(meshList[GEO_FLASHLIGHT], true);
	modelStack.PopMatrix();
}

// Update the lerp speed (in the class or Init)
float shadowLerpSpeed = 0.2f;  // Increased from 0.05f for faster pursuit

void SceneText::Render()
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

	// Update fog uniforms each frame (camera may move)
	glUniform1i(m_parameters[U_FOG_ENABLED], fogEnabled ? 1 : 0);
	glUniform1f(m_parameters[U_FOG_START], fogStart);
	glUniform1f(m_parameters[U_FOG_END], fogEnd);
	glUniform3fv(m_parameters[U_FOG_COLOR], 1, &fogColor.r);

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

	// EYEBALL TEST - isolated transformations
	modelStack.PushMatrix();
	modelStack.Translate(0.f, 100.f, -150.f);
	modelStack.Scale(20.f, 20.f, 20.f);
	modelStack.Rotate(25.f, 1.f, 0.f, 0.f);
	meshList[GEO_EYEBALL]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	meshList[GEO_EYEBALL]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
	meshList[GEO_EYEBALL]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
	meshList[GEO_EYEBALL]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_EYEBALL], true);
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

	// house
	modelStack.PushMatrix();
	{
		// spacing chosen to match the previous manual placement (50 units)
		const float start = -200.f;
		const float end = 200.f;
		const float step = 25.f;
		for (float x = start; x <= end; x += step)
		{
			modelStack.PushMatrix();
			modelStack.Translate(x, 0.f, 55.f);
			modelStack.Scale(1.f, 1.f, 1.f);
			modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
			RenderMesh(meshList[GEO_ABANDONEDHOUSE], true);
			modelStack.PopMatrix();

		}
		meshList[GEO_ABANDONEDHOUSE]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	}

	/*
	// Peter label
	modelStack.PushMatrix();
	modelStack.Translate(7.f, 0.f, 0.f);
	RenderText(meshList[GEO_TEXT], "< peter", glm::vec3(0, 1, 0));
	modelStack.PopMatrix();
	*/

	if (!isPlayerDead)
	{
		if (!isEndActivated)
		{
			modelStack.PushMatrix();
			modelStack.Translate(-200.f, 0.5f, -154.f);
			modelStack.Scale(25.f, 200.f, 25.f);
			meshList[GEO_SPARKLING_STAR]->material.kAmbient = glm::vec3(1.f, 1.f, 0.5f);
			meshList[GEO_SPARKLING_STAR]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[GEO_SPARKLING_STAR]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[GEO_SPARKLING_STAR]->material.kShininess = 5.0f;
			RenderMesh(meshList[GEO_SPARKLING_STAR], true);
			modelStack.PopMatrix();
		}

		const glm::vec3 EndminPos(-215.f, 1.0f, -165.0f);
		const glm::vec3 EndmaxPos(-195.f, 4.0f, -145.0f);

		auto inEndActivationZone = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		if (inEndActivationZone(camera.position, EndminPos, EndmaxPos))
		{
			isEndActivated = true;
		}

		if (isEndActivated)
		{
			modelStack.PushMatrix();
			modelStack.Translate(-200.f, 0.5f, -154.f);
			modelStack.Scale(25.f, 200.f, 25.f);
			meshList[GEO_SPARKLING_STAR]->material.kAmbient = glm::vec3(0.f, 1.f, 0.f);
			meshList[GEO_SPARKLING_STAR]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[GEO_SPARKLING_STAR]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
			meshList[GEO_SPARKLING_STAR]->material.kShininess = 5.0f;
			RenderMesh(meshList[GEO_SPARKLING_STAR], true);
			modelStack.PopMatrix();
		}
	}

	/*
	modelStack.PushMatrix();
	modelStack.Translate(-10.f, 1.f, 0.f);
	modelStack.Scale(1.f, 1.f, 1.f);
	meshList[GEO_PEWPEW]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	meshList[GEO_PEWPEW]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
	meshList[GEO_PEWPEW]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
	meshList[GEO_PEWPEW]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_PEWPEW], true);
	modelStack.PopMatrix();
	*/

	// tree
	modelStack.PushMatrix();
	{
		const float start = -250.f;
		const float end = 250.f;
		const float step = 21.f;
		for (float x = start; x <= end; x += step)
		{
			for (float z = start; z <= end; z += step)
			{
				modelStack.PushMatrix();
				modelStack.Translate(x, -1.f, z);
				modelStack.Scale(0.5f, 0.5f, 0.5f);
				RenderMesh(meshList[GEO_PINETREE], true);
				modelStack.PopMatrix();
			}
		}

		// material updates (if desired)
		meshList[GEO_PINETREE]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
		meshList[GEO_PINETREE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
		meshList[GEO_PINETREE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
		meshList[GEO_PINETREE]->material.kShininess = 5.0f;
	}
	modelStack.PopMatrix();

	// notes
	{
		// note
		modelStack.PushMatrix();
		modelStack.Translate(16.f, 0.6f, -1.4f);
		modelStack.Scale(1.5f, 1.5f, 1.5f);
		modelStack.Rotate(125.f, 0.f, 0.f, 1.f);
		modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(265.f, 0.f, 0.f, 1.f);
		meshList[GEO_NOTE]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
		meshList[GEO_NOTE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
		meshList[GEO_NOTE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
		meshList[GEO_NOTE]->material.kShininess = 5.0f;
		RenderMesh(meshList[GEO_NOTE], true);
		modelStack.PopMatrix();

		// note
		modelStack.PushMatrix();
		modelStack.Translate(16.5f, 4.f, 21.3f);
		modelStack.Scale(1.5f, 1.5f, 1.5f);
		modelStack.Rotate(205.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
		modelStack.Rotate(25.f, 1.f, 0.f, 0.f);
		meshList[GEO_NOTE]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
		meshList[GEO_NOTE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
		meshList[GEO_NOTE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
		meshList[GEO_NOTE]->material.kShininess = 5.0f;
		RenderMesh(meshList[GEO_NOTE], true);
		modelStack.PopMatrix();

		// note
		modelStack.PushMatrix();
		modelStack.Translate(4.f, 4.f, 41.f);
		modelStack.Scale(1.5f, 1.5f, 1.5f);
		modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(90.f, 0.f, 0.f, 1.f);
		modelStack.Rotate(25.f, 1.f, 0.f, 0.f);
		meshList[GEO_NOTE]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
		meshList[GEO_NOTE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
		meshList[GEO_NOTE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
		meshList[GEO_NOTE]->material.kShininess = 5.0f;
		RenderMesh(meshList[GEO_NOTE], true);
		modelStack.PopMatrix();
	}

	if (showDark)
	{
		if (!flashlightOn)
		{
			// how far in front of the camera
			const float distanceInFront = 15.0f;

			// camera forward vector
			glm::vec3 forward = glm::normalize(camera.target - camera.position);

			// desired world position for the quad
			glm::vec3 quadPos = camera.position + forward * distanceInFront;

			// default quad normal (your code treats quads as facing -Z by default)
			const glm::vec3 defaultNormal = glm::vec3(0.0f, 0.0f, -1.0f);

			// compute rotation to align defaultNormal -> forward
			glm::vec3 axis = glm::cross(defaultNormal, forward);
			float dotp = glm::clamp(glm::dot(defaultNormal, forward), -1.0f, 1.0f);
			float angleRad = std::acos(dotp); // angle between the vectors (radians)
			float angleDeg = glm::degrees(angleRad);

			modelStack.PushMatrix();
			{
				// translate to position in front of camera
				modelStack.Translate(quadPos.x, quadPos.y, quadPos.z);

				// apply rotation if needed
				const float eps = 1e-6f;
				if (glm::length(axis) > eps && angleDeg > 0.0001f)
				{
					axis = glm::normalize(axis);
					modelStack.Rotate(angleDeg, axis.x, axis.y, axis.z);
				}
				else if (dotp < -0.9999f)
				{
					// vectors are opposite; rotate 180 degrees around world up
					modelStack.Rotate(180.0f, 0.0f, 1.0f, 0.0f);
				}

				// scale the quad as you want
				modelStack.Scale(100.f, 100.f, 100.f);

				// set material if needed
				meshList[GEO_QUAD]->material.kAmbient = glm::vec3(0.f, 0.f, 0.f);

				// render (enable lighting if you want)
				RenderMesh(meshList[GEO_QUAD], true);
			}
			modelStack.PopMatrix();

			// flashlight text
			modelStack.PushMatrix();
			modelStack.Translate(-10.f, 2.5f, 40.f);
			modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
			modelStack.Scale(0.5f, 0.5f, 0.5f);
			RenderText(meshList[GEO_TEXT], "E to pick up", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();
		}

		if (flashlightOn)
		{
			// how far in front of the camera
			const float distanceInFront = 35.0f;

			// camera forward vector
			glm::vec3 forward = glm::normalize(camera.target - camera.position);

			// desired world position for the quad
			glm::vec3 quadPos = camera.position + forward * distanceInFront;

			// default quad normal (your code treats quads as facing -Z by default)
			const glm::vec3 defaultNormal = glm::vec3(0.0f, 0.0f, -1.0f);

			// compute rotation to align defaultNormal -> forward
			glm::vec3 axis = glm::cross(defaultNormal, forward);
			float dotp = glm::clamp(glm::dot(defaultNormal, forward), -1.0f, 1.0f);
			float angleRad = std::acos(dotp); // angle between the vectors (radians)
			float angleDeg = glm::degrees(angleRad);

			modelStack.PushMatrix();
			{
				// translate to position in front of camera
				modelStack.Translate(quadPos.x, quadPos.y, quadPos.z);

				// apply rotation if needed
				const float eps = 1e-6f;
				if (glm::length(axis) > eps && angleDeg > 0.0001f)
				{
					axis = glm::normalize(axis);
					modelStack.Rotate(angleDeg, axis.x, axis.y, axis.z);
				}
				else if (dotp < -0.9999f)
				{
					// vectors are opposite; rotate 180 degrees around world up
					modelStack.Rotate(180.0f, 0.0f, 1.0f, 0.0f);
				}

				// scale the quad as you want
				modelStack.Scale(100.f, 100.f, 100.f);

				// set material if needed
				meshList[GEO_QUAD]->material.kAmbient = glm::vec3(0.f, 0.f, 0.f);

				// render (enable lighting if you want)
				RenderMesh(meshList[GEO_QUAD], true);
			}
			modelStack.PopMatrix();
		}
	}

	RenderFlashlight();

	// UI elements (already isolated in RenderMeshOnScreen)
	//RenderMeshOnScreen(meshList[GEO_GUI], 50, 50, 10, 10);

	RenderTextOnScreen(meshList[GEO_TEXT], "Objective:", glm::vec3(1, 1, 1), 20, 15, 560);
	RenderTextOnScreen(meshList[GEO_TEXT], "- Find a way out", glm::vec3(1, 1, 1), 20, 15, 500);

	if (flashlightObjective)
	{
		RenderTextOnScreen(meshList[GEO_TEXT], "- Follow the notes to find the flashlight", glm::vec3(1, 1, 0.5), 20, 15, 530);
	}

	if (flashlightOn)
	{
		flashlightObjective = false;

		if (!isShadowSpawned)
		{

		// direct player to cutscene (the one that appears only after flash is picked up)
		modelStack.PushMatrix();
		modelStack.Translate(-5.f, 1.f, 40.f);
		modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(15.f, 0.f, 1.f, 0.f);
		RenderText(meshList[GEO_TEXT], "< < <", glm::vec3(1, 1, 1));
		modelStack.PopMatrix();

		// direct player to cutscene (the one that appears only after flash is picked up)
		modelStack.PushMatrix();
		modelStack.Translate(15.f, 1.f, 40.f);
		modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(15.f, 0.f, 1.f, 0.f);
		RenderText(meshList[GEO_TEXT], "< < <", glm::vec3(1, 1, 1));
		modelStack.PopMatrix();

		// direct player to cutscene
		modelStack.PushMatrix();
		modelStack.Translate(35.f, 1.f, 40.f);
		modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(15.f, 0.f, 1.f, 0.f);
		RenderText(meshList[GEO_TEXT], "< < <", glm::vec3(1, 1, 1));
		modelStack.PopMatrix();

		// direct player to cutscene
		modelStack.PushMatrix();
		modelStack.Translate(55.f, 1.f, 40.f);
		modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(15.f, 0.f, 1.f, 0.f);
		RenderText(meshList[GEO_TEXT], "< < <", glm::vec3(1, 1, 1));
		modelStack.PopMatrix();

		// direct player to cutscene
		modelStack.PushMatrix();
		modelStack.Translate(80.f, 1.f, 40.f);
		modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(15.f, 0.f, 1.f, 0.f);
		RenderText(meshList[GEO_TEXT], "< < <", glm::vec3(1, 1, 1));
		modelStack.PopMatrix();

		
		RenderTextOnScreen(meshList[GEO_TEXT], "- Follow the path indicators", glm::vec3(1, 1, 0.5), 20, 15, 530);
		}

		if (isShadowSpawned)
		{
			// TOWARDS END
			modelStack.PushMatrix();
			modelStack.Translate(-150.f, 4.f, -160.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-170.f, 4.f, -160.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// TOWARDS SIDE
			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, 37.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, 20.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, -1.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, -21.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, -42.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, -62.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, -82.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, -103.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(-165.f, 4.f, -123.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Rotate(90.f, 0.f, -1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// TOWARDS BACK
			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(-152.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(-130.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(-118.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(-96.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(-74.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(-52.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(-30.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(-8.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(12.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(34.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(54.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();

			// direct player
			modelStack.PushMatrix();
			modelStack.Translate(77.f, 4.f, 40.f);
			modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
			modelStack.Scale(2.f, 2.f, 2.f);
			RenderText(meshList[GEO_TEXT], "> > >", glm::vec3(1, 1, 1));
			modelStack.PopMatrix();


			RenderTextOnScreen(meshList[GEO_TEXT], "- Follow the path indicators and run", glm::vec3(1, 1, 0.5), 20, 15, 530);

			if (ChasetextTimer > 3.0f) {
				RenderTextOnScreen(meshList[GEO_TEXT], "What the fuck is that bro", glm::vec3(1, 1, 1), 25, 200, 10);
			}
			else if (ChasetextTimer > 0.0f) {
				RenderTextOnScreen(meshList[GEO_TEXT], "oh hell nah im dipping", glm::vec3(1, 1, 1), 25, 200, 10);
			}
		}

		if (flashlighttextTimer > 4.0f) {
			RenderTextOnScreen(meshList[GEO_TEXT], "You picked up a flashlight!", glm::vec3(1, 1, 0.5), 25, 200, 50);
		}
	}

	{
		if (isShadowSpawned)
		{
			runObjective = true;

			modelStack.PushMatrix();

			// Update shadow position based on timer
			if (shadowFollowTimer < 1.0f)
			{
				shadowCurrentPos = glm::vec3(90.0f, shadowY, 37.0f);
			}
			else
			{
				glm::vec3 targetPos = camera.position + glm::vec3(-5.0f, 0.0f, 0.0f);
				shadowCurrentPos.x = glm::mix(shadowCurrentPos.x, targetPos.x, shadowLerpSpeed);
				shadowCurrentPos.z = glm::mix(shadowCurrentPos.z, targetPos.z, shadowLerpSpeed);
			}

			// Translate first to position the shadow
			modelStack.Translate(shadowCurrentPos.x, shadowY, shadowCurrentPos.z);

			// Calculate direction from shadow to camera
			glm::vec3 direction = glm::normalize(camera.position - shadowCurrentPos);

			// Compute yaw angle to face the camera (around y-axis)
			float yaw = glm::degrees(atan2(direction.x, direction.z));

			// Rotate after translate to orient it at the position
			// DEBUG: Temporarily comment out to test position following (uncomment below if needed)
			modelStack.Rotate(yaw, 0.f, 1.f, 0.f);
			// If position follows, try offsets: modelStack.Rotate(yaw + 90.f, 0.f, 1.f, 0.f); or modelStack.Rotate(yaw + 180.f, 0.f, 1.f, 0.f);

			modelStack.Scale(0.5f, 0.5f, 0.5f);
			meshList[GEO_SHADOW]->material.kAmbient = glm::vec3(0.f);
			meshList[GEO_SHADOW]->material.kDiffuse = glm::vec3(1.f, 0.f, 0.f);
			meshList[GEO_SHADOW]->material.kSpecular = glm::vec3(0.f);
			meshList[GEO_SHADOW]->material.kShininess = 5.0f;

			RenderMesh(meshList[GEO_SHADOW], false);
			modelStack.PopMatrix();

			if (isPlayerDead) {

				runObjective = false;

				RenderMeshOnScreen(meshList[GEO_GUI], 1.f, 1.f, 10000.f, 10000.f);

				RenderTextOnScreen(meshList[GEO_TEXT], "YOU DIED", glm::vec3(1, 0, 0), 75, 245, 300);
				RenderTextOnScreen(meshList[GEO_TEXT], "Press ESC to exit", glm::vec3(1, 1, 0), 30, 225, 250);
			}
		}
	}

	if (isEndActivated)
	{

		runObjective = false;

		RenderMeshOnScreen(meshList[GEO_GUI], 1.f, 1.f, 10000.f, 10000.f);

		RenderTextOnScreen(meshList[GEO_TEXT], "YOU ESCAPED!", glm::vec3(1, 1, 0.3), 75, 190, 300);
		RenderTextOnScreen(meshList[GEO_TEXT], "Press ESC to exit", glm::vec3(1, 1, 0), 30, 225, 250);
	}

	if (runObjective)
	{
		RenderTextOnScreen(meshList[GEO_TEXT], "RUN", glm::vec3(1, 0, 0), 75, 325, 450);
	}

	if (startOfGame)
	{
		if (StarttextTimer > 3.0f) {
			// Show the first line for 2 seconds (from 5.0 down to 3.0)
			RenderTextOnScreen(meshList[GEO_TEXT], "This forest smells weird...", glm::vec3(1, 1, 1), 25, 200, 10);
		}
		else if (StarttextTimer > 0.0f) {
			// Show the second line for the remaining 3 seconds (from 3.0 down to 0)
			RenderTextOnScreen(meshList[GEO_TEXT], "...and sounds weird too.", glm::vec3(1, 1, 1), 25, 200, 10);
		}
	}
}

void SceneText::RenderMesh(Mesh* mesh, bool enableLight)
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


void SceneText::Exit()
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

void SceneText::HandleKeyPress(double dt)
{
	if (isPlayerDead) {
		return;  // Skip all input handling when dead
	}

	if (isEndActivated)
	{
		return;
	}

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

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_G))
	{
		// unload quad
		showDark = !showDark;
	}

	// Skip movement if stillness is active
	if (isShadowSpawned && shadowStillTimer < 1.0f)
	{
		// Still processing other keys (e.g., lighting toggles), but no movement
		return;
	}

	// Calculate forward and right vectors based on camera orientation
	glm::vec3 forward = glm::normalize(camera.target - camera.position);
	glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));

	// Use IsKeyDown for continuous movement while holding the key
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W))
	{
		// Move forward
		float movement = moveSpeed * static_cast<float>(dt);
		camera.position += forward * movement;
		camera.target += forward * movement;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
	{
		// Move backward
		float movement = moveSpeed * static_cast<float>(dt);
		camera.position -= forward * movement;
		camera.target -= forward * movement;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A))
	{
		// Move left (strafe)
		float movement = moveSpeed * static_cast<float>(dt);
		camera.position -= right * movement;
		camera.target -= right * movement;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D))
	{
		// Move right (strafe)
		float movement = moveSpeed * static_cast<float>(dt);
		camera.position += right * movement;
		camera.target += right * movement;
	}

	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT))
	{
		// sprint at 10.f
		float movement = (1.5 * moveSpeed) * static_cast<float>(dt);
		camera.position += forward * movement;
		camera.target += forward * movement;
	}

	// Check if the player is currently moving (used for bobbing)
	bool isMoving = KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W) ||
		KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S) ||
		KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A) ||
		KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D);

	// Apply camera bobbing if shadow is spawned and player is moving
	if (isShadowSpawned && isMoving)
	{
		bobTime += static_cast<float>(dt) * bobSpeed;  // Accumulate bobbing time
		float bobOffset = glm::sin(bobTime) * bobAmount;  // Calculate vertical offset
		camera.position.y += bobOffset;
		camera.target.y += bobOffset;
	}

	// Adjust clamping limits to account for bobbing range (prevents getting stuck)
	float minY = 3.2f - (isShadowSpawned ? bobAmount : 0.0f);
	float maxY = 3.3f + (isShadowSpawned ? bobAmount : 0.0f);

	// Clamp camera height to adjusted limits
	if (camera.position.y < minY) {
		camera.position.y = minY;
		if (camera.target.y < minY)
			camera.target.y = minY;
	}
	if (camera.position.y > maxY) {
		camera.position.y = maxY;
		if (camera.target.y > maxY)
			camera.target.y = maxY;
	}

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E))
	{
		const glm::vec3 minPos(-20.f, 1.0f, 35.0f);
		const glm::vec3 maxPos(0.f, 4.0f, 41.0f);

		auto isInsideBox = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) {
			return (p.x >= mn.x && p.x <= mx.x) &&
				(p.y >= mn.y && p.y <= mx.y) &&
				(p.z >= mn.z && p.z <= mx.z);
			};

		if (isInsideBox(camera.position, minPos, maxPos))
		{

			flashlightOn = true;

			if (light[0].power <= 0.3f)
				light[0].power = 0.3f;
			else
				light[0].power = 0.0f;

			// upload the changed power to the shader
			glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
		}
	}
}