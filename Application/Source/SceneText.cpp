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
	m_programID = LoadShaders("Shader//Texture.vertexshader", "Shader//Texture.fragmentshader");
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
	if (camera.position.y < 3.0f) {
		camera.position.y = 3.0f;
		// keep the camera looking at the same relative height (adjust target to avoid looking too far down)
		if (camera.target.y < 3.0f) camera.target.y = 3.0f;
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

	meshList[GEO_ZUL] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_ZUL]->textureID = LoadTGA("Images//zulmobile.tga");

	meshList[GEO_GRASS] = MeshBuilder::GenerateQuad("Quad", glm::vec3(1.f, 1.f, 1.f), 10.f);
	meshList[GEO_GRASS]->textureID = LoadTGA("Images//coast_sand_rocks_02 copy.tga");

	//meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	//meshList[GEO_GUI]->textureID = LoadTGA("Images//color.tga");

	// 16 x 16 is the number of columns and rows for the text
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//calibri.tga");

	meshList[GEO_GUI] = MeshBuilder::GenerateQuad("GUI", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_GUI]->textureID = LoadTGA("Images//zulmobile.tga");

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

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);


	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(0, 200, 0);
	light[0].color = glm::vec3(1, 0, 0);
	light[0].type = Light::LIGHT_DIRECTIONAL;
	light[0].power = 0.3;
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
}

void SceneText::HandleMouseInput() {
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
	modelStack.Scale(100, 100, 1);

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
			glm::vec3(i * 1.0f, 0, 0));
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
	modelStack.Translate(0.f, 25.f, 0.f);
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
		meshList[GEO_GRASS]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
	}
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(5.f, 0.f, 55.f);
	modelStack.Scale(1.f, 1.f, 1.f);
	modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
	meshList[GEO_ABANDONEDHOUSE]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	meshList[GEO_ABANDONEDHOUSE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
	meshList[GEO_ABANDONEDHOUSE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
	meshList[GEO_ABANDONEDHOUSE]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_ABANDONEDHOUSE], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(25.f, 0.f, 55.f);
	modelStack.Scale(1.f, 1.f, 1.f);
	modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
	meshList[GEO_ABANDONEDHOUSE2]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	meshList[GEO_ABANDONEDHOUSE2]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
	meshList[GEO_ABANDONEDHOUSE2]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
	meshList[GEO_ABANDONEDHOUSE2]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_ABANDONEDHOUSE2], true);
	modelStack.PopMatrix();

	/*
	// Peter label
	modelStack.PushMatrix();
	modelStack.Translate(7.f, 0.f, 0.f);
	RenderText(meshList[GEO_TEXT], "< peter", glm::vec3(0, 1, 0));
	modelStack.PopMatrix();
	*/


	// Tower
	modelStack.PushMatrix();
	modelStack.Translate(-10.f, -3.f, 0.f);
	modelStack.Scale(5.f, 5.f, 5.f);
	meshList[GEO_TOWER]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	meshList[GEO_TOWER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
	meshList[GEO_TOWER]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
	meshList[GEO_TOWER]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_TOWER], true);
	modelStack.PopMatrix();

	/*
	// grass 3D
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


	/*
	// Tower
	modelStack.PushMatrix();
	modelStack.Translate(10.f, -3.f, 0.f);
	modelStack.Scale(5.f, 5.f, 5.f);
	meshList[GEO_TREETREE]->material.kAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	meshList[GEO_TREETREE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
	meshList[GEO_TREETREE]->material.kSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
	meshList[GEO_TREETREE]->material.kShininess = 5.0f;
	RenderMesh(meshList[GEO_TREETREE], true);
	modelStack.PopMatrix();
	*/

	// UI elements (already isolated in RenderMeshOnScreen)
	RenderMeshOnScreen(meshList[GEO_GUI], 50, 50, 10, 10);

	RenderTextOnScreen(meshList[GEO_TEXT], "work in progress", glm::vec3(1, 1, 1), 25 /*size*/, 100 /*horizontal pos*/, 10 /*vertical pos*/);

	// Text in world space
	modelStack.PushMatrix();
	modelStack.Translate(25.f, 10.f, 0.f);
	modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
	RenderText(meshList[GEO_TEXT], "shen me zai shang mian", glm::vec3(0, 1, 0));
	modelStack.PopMatrix();
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

	// Movement speed (units per second)
	const float moveSpeed = 5.0f;

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

	// Clamp camera height so it never goes below ground (y = 3.0f)
	if (camera.position.y < 3.0f) {
		camera.position.y = 3.0f;
		if (camera.target.y < 3.0f)
			camera.target.y = 3.0f;
		// Re-init to refresh internal vectors after adjustment
		camera.Init(camera.position, camera.target, camera.up);
	}

	if (camera.position.y > 3.1f) {
		camera.position.y = 3.1f;
		if (camera.target.y > 3.1f)
			camera.target.y = 3.1f;
		// Re-init to refresh internal vectors after adjustment
		camera.Init(camera.position, camera.target, camera.up);
	}
}