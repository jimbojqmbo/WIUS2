#include "evochat.h"
#include "GL\glew.h"

#include "shader.hpp"
#include "Application.h"
#include "MeshBuilder.h"
#include "KeyboardController.h"
#include "Light.h"
#include "Material.h"
#include "GLFW\glfw3.h"

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_inverse.hpp>

evochat::evochat()
{
}

evochat::~evochat()
{
}

void evochat::Init()
{

	camera.Init(45.f, 45.f, 10.f);

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	projectionStack.LoadMatrix(projection);

	moonRotation = 0.0f;

	currAnim = ANIM_SUN;

	moonRotation = 0.0f;
	earthRotation = 0.0f;
	sunRotation = 0.0f;

	// Set background color to dark blue
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	//Enable depth buffer and depth testing
	glEnable(GL_DEPTH_TEST);

	//Enable back face culling
	glEnable(GL_CULL_FACE);

	//Default to fill mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Generate a default VAO for now
	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);

	/* Load the shader programs
	m_programID = LoadShaders("Shader//TransformVertexShader.vertexshader",
								"Shader//SimpleFragmentShader.fragmentshader");*/

	// Load the shader programs
	m_programID = LoadShaders("Shader//Shading.vertexshader", "Shader//LightSource.fragmentshader");

	glUseProgram(m_programID);

	// Get a handle for our "MVP" uniform
	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");

	// Add the following codes
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
	// Step 11 - add these
	m_parameters[U_LIGHT0_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[0].spotDirection");
	m_parameters[U_LIGHT0_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[0].cosCutoff");
	m_parameters[U_LIGHT0_COSINNER] = glGetUniformLocation(m_programID, "lights[0].cosInner");
	m_parameters[U_LIGHT0_EXPONENT] = glGetUniformLocation(m_programID, "lights[0].exponent");
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled"); light[0].cosCutoff = 45.f;
	light[0].cosInner = 30.f;
	light[0].exponent = 3.f;
	light[0].spotDirection = glm::vec3(0.f, 1.f, 0.f);
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled");

	// Init VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = nullptr;
	}

	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
	meshList[GEO_SPHERE] = MeshBuilder::GenerateSphere("Light", glm::vec3(1.f, 1.f, 1.f), 1.f, 16, 16);
	meshList[GEO_CYLINDER] = MeshBuilder::GenerateCylinder("Cylinder", glm::vec3(0.6f, 0.7f, 0.8f), 36, 1.f, 5.f);
	meshList[GEO_HEMISPHERE] = MeshBuilder::GenerateHemisphere("Hemisphere", glm::vec3(0.6f, 0.7f, 0.8f), 18, 36, 1.f);
	meshList[GEO_HEAD] = MeshBuilder::GenerateHead("Head", glm::vec3(0.6f, 0.7f, 0.8f), 20, 36, 1.f, 2.5f);

	light[0].type = Light::LIGHT_POINT;
	light[0].position = glm::vec3(0, 5, 0);
	light[0].color = glm::vec3(1, 1, 1);
	light[0].power = 1;
	light[0].kC = 1.f;
	light[0].kL = 0.01f;
	light[0].kQ = 0.001f;

	glUniform1i(m_parameters[U_LIGHT0_TYPE], light[0].type);
	glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, glm::value_ptr(light[0].color));
	glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	glUniform1f(m_parameters[U_LIGHT0_KC], light[0].kC);
	glUniform1f(m_parameters[U_LIGHT0_KL], light[0].kL);
	glUniform1f(m_parameters[U_LIGHT0_KQ], light[0].kQ);

	enableLight = true;

	// Step 14 - Add the following codes to // pass the spotlight properties to shader
	glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF], cosf(glm::radians<float>(light[0].cosCutoff)));
	glUniform1f(m_parameters[U_LIGHT0_COSINNER], cosf(glm::radians<float>(light[0].cosInner)));
	glUniform1f(m_parameters[U_LIGHT0_EXPONENT], light[0].exponent);
}

void evochat::Update(double dt)
{
	HandleKeyPress();

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

	moonRotation += static_cast<float>(dt) * 20.f;

	if (animPlaying)
		animTime += dt;

	switch (currentAnim)
	{
	case ANIM_IDLE:
		AnimateIdle();
		break;

	case ANIM_WAVE:
		AnimateWave();
		break;

	case ANIM_ATTACK:
		AnimateAttack();
		break;

	case ANIM_SOMERSAULT:
		AnimateSomersault();
		break;
	}

	if (idleActive)
	{
		idleTime += dt;
	}
}

void evochat::AnimateIdle()
{
	if (currentAnim != ANIM_IDLE)
		return;

	float breath = sin(animTime * 2.0f) * 0.1f;
	float bob = sin(animTime * 1.5f) * 0.05f;

	thoraxTranslation = breath;
	headTranslation = bob;

	leftShoulderRotation = sin(animTime * 1.3f) * 5;
	rightShoulderRotation = sin(animTime * 1.3f + 3.14f) * 5;
}

void evochat::AnimateWave()
{
	// Loop the wave (period 2 seconds)
	float t = animTime;
	float swing = sin(t * 4.0f) * 35.f; // big arm swing ±35 degrees

	// Right arm only
	rightClavicleRotation = -100.f;          // shoulder joint
	rightShoulderRotation = -100.f * 0.7f;   // forearm follow-through

	float shoulderSwing = glm::clamp(swing, -50.f, swing);
	rightShoulderRotation = shoulderSwing;

	// Slight body twist
	//pelvisRotation = sin(t * 2.5f) * 5.f;

	// Optional: stop after 4 seconds
	if (t > 4.0f) {
		animPlaying = false;
		currentAnim = ANIM_NONE;
	}
}

void evochat::AnimateAttack()
{
	float t = animTime;
	float duration = 0.7f;
	float phase = glm::clamp(t / duration, 0.f, 1.f);

	float punch = sin(phase * 3.14159f);

	// STOP HEAD BOBBING
	headTranslation = 0.f;

	// torso: push straight forward (NO SIDEWAYS)
	thoraxTranslation = punch * 0.3f;   // moves forward along Z or Y depending on your model
	pelvisRotation = punch * 0.f;    // NO SIDE TWIST

	// shoulders: both move symmetrically forward/back
	float shoulderPullBack = -punch * 20.f;   // pull back equally
	float shoulderExtend = punch * 60.f;    // punch forward equally

	leftClavicleRotation = shoulderPullBack + shoulderExtend;
	rightClavicleRotation = shoulderPullBack + shoulderExtend;

	// upper arms: punch forward, no sideways rotation
	leftShoulderRotation = punch * -70.f;   // arms go straight forward
	rightShoulderRotation = punch * -70.f;

	// forearms: extend straight forward
	leftForearmRotation = punch * -90.f;
	rightForearmRotation = punch * -90.f;

	// hands move slightly forward during punch
	leftForearmTranslation = punch * 0.05f;
	rightForearmTranslation = punch * 0.05f;

	// Lean back slightly as the leg kicks forward
	thoraxRotationX = punch * 6.f;       // negative X = forward lean, positive = backward lean
	thoraxTranslationY = -punch * 0.2f;     // slight dip

	// RIGHT LEG (KICKING LEG)
	// Hip swings forward (robot faces +Z → negative X rotation moves forward)
	rightHipRotation = punch * -25.f;

	// upper leg lifts
	rightThighRotation = punch * -55.f;

	// tibia snaps forward
	rightTibiaRotation = punch * -85.f;

	// foot points forward
	rightFeetRotation = punch * 20.f;

	if (t > duration)
	{
		animPlaying = false;
		currentAnim = ANIM_NONE;
	}
}

void evochat::AnimateSomersault()
{
	float t = animTime;
	float duration = 1.2f;
	float phase = glm::clamp(t / duration, 0.f, 1.f);

	float jumpY = sin(phase * 3.14159f) * 2.0f;
	pelvisTranslation = jumpY;

	pelvisRotation = phase * 360.f;

	leftShoulderRotation = -20.f;
	rightShoulderRotation = 20.f;
	leftHipRotation = 15.f;
	rightHipRotation = -15.f;

	if (t > duration)
	{
		animPlaying = false;
		currentAnim = ANIM_NONE;
	}
}

void evochat::Render()
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

	// Calculate the light position in camera space
	glm::vec3 lightPosition_cameraspace = viewStack.Top() * glm::vec4(light[0].position, 1);
	glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, glm::value_ptr(lightPosition_cameraspace));

	if (light[0].type == Light::LIGHT_DIRECTIONAL)
	{
		// for directional light
		if (light[0].type == Light::LIGHT_DIRECTIONAL)
		{
			glm::vec3 lightDir(light[0].position.x,
				light[0].position.y, light[0].position.z);
			glm::vec3 lightDirection_cameraspace =
				viewStack.Top() * glm::vec4(lightDir, 0);
			glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1,
				glm::value_ptr(lightDirection_cameraspace));
		}
	}
	// Step 14 - logic for spotlight
	else if (light[0].type == Light::LIGHT_SPOT) {
		glm::vec3 lightPosition_cameraspace =
			viewStack.Top() * glm::vec4(light[0].position, 1);
		glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1,
			glm::value_ptr(lightPosition_cameraspace));
		glm::vec3 spotDirection_cameraspace =
			viewStack.Top() * glm::vec4(light[0].spotDirection, 0);
		glUniform3fv(m_parameters[U_LIGHT0_SPOTDIRECTION],
			1, glm::value_ptr(spotDirection_cameraspace));
	}
	else {
		// for point light
		{
			// Calculate the light position in camera space
			glm::vec3 lightPosition_cameraspace =
				viewStack.Top() * glm::vec4(light[0].position, 1);
			glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1,
				glm::value_ptr(lightPosition_cameraspace));
		}
	}

	/* Calculate the Model - View - Projection matrix
	glm::mat4 MVP = projectionStack.Top() * viewStack.Top() *
		modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE,
		glm::value_ptr(MVP));*/

	modelStack.PushMatrix();
	// Render objects using new RenderMesh()
	RenderMesh(meshList[GEO_AXES], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	// Render light
	modelStack.Translate(light[0].position.x, light[0].position.y, light[0].position.z);
	modelStack.Scale(0.1f, 0.1f, 0.1f);
	RenderMesh(meshList[GEO_SPHERE], false);
	modelStack.PopMatrix();

	{

		// pelvis location
		modelStack.PushMatrix();
		modelStack.Translate(0.f, pelvisTranslation, 0.f);
		modelStack.Rotate(pelvisRotation, 1.f, 0.f, 0.f);


		{
			// thorax
			// note: body is +2.8f height from pelvis
			
			modelStack.PushMatrix();
			modelStack.Translate(0.f, 2.8f, 0.f);
			//modelStack.Rotate(pelvisRotation, pelvisRotation, pelvisRotation, pelvisRotation);

			{
				// head
				modelStack.PushMatrix();
				modelStack.Translate(0.f, 5.f, 0.f);
				//modelStack.Rotate

				{
					// left eye
					modelStack.PushMatrix();
					modelStack.Translate((headTranslation - 0.4f), (headTranslation - 0.5f), (headTranslation + 0.9f));
					//modelStack.Rotate

					{
						// left pupil
						modelStack.PushMatrix();
						modelStack.Translate(leftEyeTranslation, leftEyeTranslation, (leftEyeTranslation + 0.4f));

						// pop pupil
						modelStack.Scale(0.1f, 0.12f, 0.1f);
						meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
						meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
						meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
						meshList[GEO_SPHERE]->material.kShininess = 5.0f;
						RenderMesh(meshList[GEO_SPHERE], true);
						modelStack.PopMatrix();
					}

					// pop eye
					modelStack.Scale(0.4f, 0.5f, 0.4f);
					meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
					meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.992f, 0.992f, 0.588f);
					meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
					meshList[GEO_SPHERE]->material.kShininess = 5.0f;
					RenderMesh(meshList[GEO_SPHERE], true);
					modelStack.PopMatrix();
				}

				{
					// right eye
					modelStack.PushMatrix();
					modelStack.Translate((headTranslation + 0.4f), (headTranslation - 0.5f), (headTranslation + 0.9f));
					//modelStack.Rotate

					{
						// right pupil
						modelStack.PushMatrix();
						modelStack.Translate(rightEyeTranslation, rightEyeTranslation, (rightEyeTranslation + 0.4f));

						// pop pupil
						modelStack.Scale(0.1f, 0.12f, 0.1f);
						meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
						meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.f, 0.f, 0.f);
						meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
						meshList[GEO_SPHERE]->material.kShininess = 5.0f;
						RenderMesh(meshList[GEO_SPHERE], true);
						modelStack.PopMatrix();
					}

					// pop eye
					modelStack.Scale(0.4f, 0.5f, 0.4f);
					meshList[GEO_SPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
					meshList[GEO_SPHERE]->material.kDiffuse = glm::vec3(0.992f, 0.992f, 0.588f);
					meshList[GEO_SPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
					meshList[GEO_SPHERE]->material.kShininess = 5.0f;
					RenderMesh(meshList[GEO_SPHERE], true);
					modelStack.PopMatrix();
				}

				// pop head
				modelStack.Scale(0.9f, 1.f, 0.9f);
				meshList[GEO_HEAD]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
				meshList[GEO_HEAD]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[GEO_HEAD]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
				meshList[GEO_HEAD]->material.kShininess = 5.0f;
				RenderMesh(meshList[GEO_HEAD], true);
				modelStack.PopMatrix();
			}

			{
				// left clavicle (shoulder)
				// note: clavicle is +2.2f height from thorax
				modelStack.PushMatrix();
				// Fixed position: clavicle is 1.2 left, 2.2 up from thorax
				modelStack.Translate(-1.2f, 2.2f, 0.f);
				modelStack.Rotate(leftClavicleRotation, 1.f, 0.f, 0.f);

				{
					// left real shoulder
					// note: shoulder height is -0.7f from clavicle

					modelStack.PushMatrix();
					// Shoulder sits slightly below clavicle (-0.7)
					modelStack.Translate(0.f, -0.7f, 0.f);
					modelStack.Rotate(leftShoulderRotation, 1.f, 0.f, 0.f);

					{
						// left forearm
						// note: forearm is -1.6f height from shoulder

						modelStack.PushMatrix();
						// Forearm is fixed offset below shoulder
						modelStack.Translate(0.f, -1.6f, 0.f);
						modelStack.Rotate(leftForearmRotation, 1.f, 0.f, 0.f);

						{
							// left palm
							
							modelStack.PushMatrix();
							modelStack.Translate(leftForearmTranslation, (leftForearmTranslation - 1.f), leftForearmTranslation);
							//modelStack.Rotate(5.f, leftForearmRotation, leftForearmRotation, leftForearmRotation);

							// pop palm
							modelStack.Scale(0.3f, 0.25f, 0.3f);
							meshList[GEO_HEMISPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
							meshList[GEO_HEMISPHERE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
							meshList[GEO_HEMISPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
							meshList[GEO_HEMISPHERE]->material.kShininess = 5.0f;
							RenderMesh(meshList[GEO_HEMISPHERE], true);
							modelStack.PopMatrix();
						}

						// pop forearm
						modelStack.Scale(0.2f, 0.35f, 0.2f);
						meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
						meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
						meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
						meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
						RenderMesh(meshList[GEO_CYLINDER], true);
						modelStack.PopMatrix();
					}

					// pop shoulder
					modelStack.Scale(0.2f, 0.35f, 0.2f);
					meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
					meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
					meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
					meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
					RenderMesh(meshList[GEO_CYLINDER], true);
					modelStack.PopMatrix();
				}

				// pop clavicle
				modelStack.Scale(0.1f, 0.1f, 0.1f);
				RenderMesh(meshList[GEO_SPHERE], true);
				modelStack.PopMatrix();
			}

			{
				// right clavicle (shoulder)
				// note: clavicle is +2.2f height from thorax
				modelStack.PushMatrix();
				modelStack.Translate(1.2f, 2.2f, 0.f);
				modelStack.Rotate(rightClavicleRotation, 1.f, 0.f, 0.f);

				{
					// right real shoulder
					// note: shoulder height is -0.7f from clavicle

					modelStack.PushMatrix();
					modelStack.Translate(0.f, -0.7f, 0.f);
					modelStack.Rotate(rightShoulderRotation, 1.f, 0.f, 0.f);

					{
						// right forearm
						// note: forearm is -1.6f height from shoulder

						modelStack.PushMatrix();
						modelStack.Translate(0.f, -1.6f, 0.f);
						modelStack.Rotate(rightForearmRotation, 1.f, 0.f, 0.f);

						{
							// right palm

							modelStack.PushMatrix();
							modelStack.Translate(rightForearmTranslation, (rightForearmTranslation - 1.f), rightForearmTranslation);
							//modelStack.Rotate(5.f, leftForearmRotation, leftForearmRotation, leftForearmRotation);

							// pop palm
							modelStack.Scale(0.3f, 0.25f, 0.3f);
							meshList[GEO_HEMISPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
							meshList[GEO_HEMISPHERE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
							meshList[GEO_HEMISPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
							meshList[GEO_HEMISPHERE]->material.kShininess = 5.0f;
							RenderMesh(meshList[GEO_HEMISPHERE], true);
							modelStack.PopMatrix();
						}

						// pop forearm
						modelStack.Scale(0.2f, 0.35f, 0.2f);
						meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
						meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
						meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
						meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
						RenderMesh(meshList[GEO_CYLINDER], true);
						modelStack.PopMatrix();
					}

					// pop shoulder
					modelStack.Scale(0.2f, 0.35f, 0.2f);
					meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
					meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
					meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
					meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
					RenderMesh(meshList[GEO_CYLINDER], true);
					modelStack.PopMatrix();
				}

				// pop clavicle
				modelStack.Scale(0.1f, 0.1f, 0.1f);
				RenderMesh(meshList[GEO_SPHERE], true);
				modelStack.PopMatrix();
			}

			// pop body
			modelStack.Scale(1.2f, 1.2f, 1.2f);
			meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
			meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
			meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
			meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
			RenderMesh(meshList[GEO_CYLINDER], true);
			modelStack.PopMatrix();
		}

		{

			// right hip
			modelStack.PushMatrix();
			modelStack.Translate(0.6f, -0.2f, 0.f);
			modelStack.Rotate(rightHipRotation, 1.f, 0.f, 0.f);

			{

				// left thigh
				// note: put thigh y-axis as 0.4f
				modelStack.PushMatrix();
				modelStack.Translate(0.f, -0.4f, 0.f);     // fixed offset from hip
				modelStack.Translate(rightHipTranslation, 0.f, 0.f);  // animation, if any
				modelStack.Rotate(rightThighRotation, 1.f, 0.f, 0.f);

				{
					// right tibia
					// note: put tibia y-axis as -2.4f
					modelStack.PushMatrix();
					modelStack.Translate(rightThighTranslation, (rightThighTranslation - 2.4f), (rightThighTranslation));
					modelStack.Rotate(rightTibiaRotation, 1.f, 0.f, 0.f);

					{
						// right feet
						// note: set feet y-axis as -1.8f
						modelStack.PushMatrix();
						modelStack.Translate(rightTibiaTranslation, (rightTibiaTranslation - 1.8f), rightTibiaTranslation);
						modelStack.Rotate(rightFeetRotation, 1.f, 0.f, 0.f);
						
						// pop feet
						modelStack.Scale(0.6f, 0.6f, 0.6f);
						meshList[GEO_HEMISPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
						meshList[GEO_HEMISPHERE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
						meshList[GEO_HEMISPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
						meshList[GEO_HEMISPHERE]->material.kShininess = 5.0f;
						RenderMesh(meshList[GEO_HEMISPHERE], true);
						modelStack.PopMatrix();
					}

					// pop tibia
					modelStack.Scale(0.2f, 0.5f, 0.2f);
					meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
					meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
					meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
					meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
					RenderMesh(meshList[GEO_CYLINDER], true);
					modelStack.PopMatrix();
				}

				// pop thigh
				modelStack.Scale(0.2f, 0.5f, 0.2f);
				meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
				meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
				meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
				meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
				RenderMesh(meshList[GEO_CYLINDER], true);
				modelStack.PopMatrix();

			}

			modelStack.Scale(0.1f, 0.1f, 0.1f);
			RenderMesh(meshList[GEO_SPHERE], true);
			modelStack.PopMatrix();
		}

		{

			// left hip
			modelStack.PushMatrix();
			modelStack.Translate(-0.6f, -0.2f, 0.f);
			//modelStack.Rotate(pelvisRotation, pelvisRotation, pelvisRotation, pelvisRotation);

				{

					// left thigh
					// note: put thigh y-axis as -0.4f
					modelStack.PushMatrix();
					modelStack.Translate(0.f, -0.4f, 0.f);     // fixed offset from hip
					modelStack.Translate(leftHipTranslation, 0.f, 0.f);  // animation, if any
					modelStack.Rotate(leftThighRotation, 1.f, 0.f, 0.f);

					{
						// left tibia
						// note: put tibia y-axis as -2.4f
						modelStack.PushMatrix();
						modelStack.Translate(leftThighTranslation, (leftThighTranslation - 2.4f), (leftThighTranslation));
						modelStack.Rotate(leftTibiaRotation, 1.f, 0.f, 0.f);

						{
							// right feet
							// note: set feet y-axis as -2.3f
							modelStack.PushMatrix();
							modelStack.Translate(leftTibiaTranslation, (leftTibiaTranslation - 1.8f), leftTibiaTranslation);
							modelStack.Rotate(leftFeetRotation, 1.f, 0.f, 0.f);

							// pop feet
							modelStack.Scale(0.6f, 0.6f, 0.6f);
							meshList[GEO_HEMISPHERE]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
							meshList[GEO_HEMISPHERE]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
							meshList[GEO_HEMISPHERE]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
							meshList[GEO_HEMISPHERE]->material.kShininess = 5.0f;
							RenderMesh(meshList[GEO_HEMISPHERE], true);
							modelStack.PopMatrix();
						}

						// pop tibia
						modelStack.Scale(0.2f, 0.5f, 0.2f);
						meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
						meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
						meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
						meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
						RenderMesh(meshList[GEO_CYLINDER], true);
						modelStack.PopMatrix();
					}

					// pop thigh
					modelStack.Scale(0.2f, 0.5f, 0.2f);
					meshList[GEO_CYLINDER]->material.kAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
					meshList[GEO_CYLINDER]->material.kDiffuse = glm::vec3(0.6f, 0.6f, 0.6f);
					meshList[GEO_CYLINDER]->material.kSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
					meshList[GEO_CYLINDER]->material.kShininess = 5.0f;
					RenderMesh(meshList[GEO_CYLINDER], true);
					modelStack.PopMatrix();

				}

			// pop left hip
			modelStack.Scale(0.1f, 0.1f, 0.1f);
			RenderMesh(meshList[GEO_SPHERE], true);
			modelStack.PopMatrix();

		}

		// pop for pelvis
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		RenderMesh(meshList[GEO_SPHERE], true);
		modelStack.PopMatrix();
	}

}

void evochat::ResetAllTransforms()
{
	// Master timer reset
	animTime = 0.f;

	// ------------------------------
	// PELVIS (root joint)
	// ------------------------------
	pelvisTranslation = 0.f;
	pelvisRotation = 0.f;

	// ------------------------------
	// THORAX & HEAD
	// ------------------------------
	thoraxTranslation = 0.f;
	headTranslation = 0.f;

	// ------------------------------
	// EYES
	// ------------------------------
	leftEyeTranslation = 0.f;
	rightEyeTranslation = 0.f;

	// ------------------------------
	// ARMS (Left)
	// ------------------------------
	leftClavicleTranslation = 0.f;
	leftClavicleRotation = 0.f;

	leftShoulderTranslation = 0.f;
	leftShoulderRotation = 0.f;

	leftForearmTranslation = 0.f;
	leftForearmRotation = 0.f;

	// ------------------------------
	// ARMS (Right)
	// ------------------------------
	rightClavicleTranslation = 0.f;
	rightClavicleRotation = 0.f;

	rightShoulderTranslation = 0.f;
	rightShoulderRotation = 0.f;

	rightForearmTranslation = 0.f;
	rightForearmRotation = 0.f;

	// ------------------------------
	// LEGS (Left)
	// ------------------------------
	leftHipTranslation = 0.f;
	leftHipRotation = 0.f;

	leftThighTranslation = 0.f;
	leftThighRotation = 0.f;

	leftTibiaTranslation = 0.f;
	leftTibiaRotation = 0.f;

	// ------------------------------
	// LEGS (Right)
	// ------------------------------
	rightHipTranslation = 0.f;
	rightHipRotation = 0.f;

	rightThighTranslation = 0.f;
	rightThighRotation = 0.f;

	rightTibiaTranslation = 0.f;
	rightTibiaRotation = 0.f;
}

void evochat::RenderMesh(Mesh* mesh, bool enableLight)
{
	glm::mat4 MVP, modelView, modelView_inverse_transpose;

	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();

	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));

	modelView = viewStack.Top() * modelStack.Top();

	glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, glm::value_ptr(modelView));
	if (enableLight)
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 1);

		modelView_inverse_transpose = glm::inverseTranspose(modelView);

		glUniformMatrix4fv(m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE], 1, GL_FALSE, glm::value_ptr(modelView_inverse_transpose));

		//load material
		glUniform3fv(m_parameters[U_MATERIAL_AMBIENT], 1, glm::value_ptr(mesh -> material.kAmbient));
		glUniform3fv(m_parameters[U_MATERIAL_DIFFUSE], 1, glm::value_ptr(mesh -> material.kDiffuse));
		glUniform3fv(m_parameters[U_MATERIAL_SPECULAR], 1, glm::value_ptr(mesh -> material.kSpecular));
		glUniform1f(m_parameters[U_MATERIAL_SHININESS], mesh->material.kShininess);
	}
	else
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	}

	mesh->Render();

}


void evochat::Exit()
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

void evochat::HandleKeyPress() 
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

	if (KeyboardController::GetInstance()->IsKeyPressed('E'))
	{
		// Only animate the earth
		currAnim = ANIM_EARTH;
	}

	if (KeyboardController::GetInstance()->IsKeyPressed('M'))
	{
		// Only aniate the moon
		currAnim = ANIM_MOON;
	}

	if (KeyboardController::GetInstance() -> IsKeyPressed(GLFW_KEY_ENTER))
	{
		// Only animate the sun
		currAnim = ANIM_SUN;
	}

	if (KeyboardController::GetInstance()->IsKeyPressed('R'))
	{
		// Reset everything to original state
		currAnim = ANIM_SUN;
		moonRotation = 0.0f;
		earthRotation = 0.0f;
		sunRotation = 0.0f;
	}

	if (KeyboardController::GetInstance() -> IsKeyPressed(GLFW_KEY_0))
	{
		// Toggle light on or off
		enableLight = !enableLight;
	}

	if (Application::IsKeyPressed('Z'))
	{
		idleActive = true;
	}

	if (Application::IsKeyPressed('Z')) {
		currentAnim = ANIM_IDLE;
		animPlaying = true;
	}

	if (Application::IsKeyPressed('X')) {
		currentAnim = ANIM_WAVE;
		animTime = 0.f;
		animPlaying = true;
	}

	if (Application::IsKeyPressed('C')) {
		currentAnim = ANIM_ATTACK;
		animTime = 0.f;
		animPlaying = true;
	}

	if (Application::IsKeyPressed('V')) {
		currentAnim = ANIM_SOMERSAULT;
		animTime = 0.f;
		animPlaying = true;
	}

	if (Application::IsKeyPressed('R')) {
		currentAnim = ANIM_NONE;
		animTime = 0.f;
		animPlaying = false;
		ResetAllTransforms();   // You should implement resetting your variables
	}

	if (light[0].type == Light::LIGHT_POINT) {
		light[0].type = Light::LIGHT_DIRECTIONAL;
	}
	else {
		light[0].type = Light::LIGHT_POINT;
	}

	glUniform1i(m_parameters[U_LIGHT0_TYPE], light[0].type);
}
