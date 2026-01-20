#ifndef SCENE_LIGHT_H
#define SCENE_LIGHT_H

#include "Scene.h"
#include "Mesh.h"

#include "AltAzCamera.h"
#include "MatrixStack.h"
#include "Light.h"

class SceneLight : public Scene
{
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_SPHERE,
		GEO_LIGHT,

		GEO_HEAD,
		GEO_CYLINDER,
		GEO_HEMISPHERE,
		NUM_GEOMETRY,
	};

	enum UNIFORM_TYPE
	{
		U_MVP = 0,
		// Add the following constants
		U_MODELVIEW,
		U_MODELVIEW_INVERSE_TRANSPOSE,
		U_MATERIAL_AMBIENT,
		U_MATERIAL_DIFFUSE,
		U_MATERIAL_SPECULAR,
		U_MATERIAL_SHININESS,
		U_LIGHT0_POSITION,
		U_LIGHT0_COLOR,
		U_LIGHT0_POWER,
		U_LIGHT0_KC,
		U_LIGHT0_KL,
		U_LIGHT0_KQ,
		U_LIGHTENABLED,
		U_TOTAL,
	};

	enum ANIMATION
	{
		ANIM_MOON,
		ANIM_EARTH,
		ANIM_SUN,

		ANIM_NONE,
		ANIM_IDLE,
		ANIM_WAVE,
		ANIM_ATTACK,
		ANIM_SOMERSAULT,

		NUM_ANIM
	};
	
	ANIMATION currentAnim = ANIM_NONE;
	float animTime = 0.f;
	bool animPlaying = false;

	// Keep track of current animation
	ANIMATION currAnim;

	SceneLight();
	~SceneLight();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	void HandleKeyPress();
	void RenderMesh(Mesh* mesh, bool enableLight);

	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];

	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	int projType = 1; // fix to 0 for orthographic, 1 for projection

	AltAzCamera camera;

	MatrixStack modelStack, viewStack, projectionStack;

	float moonRotation;
	float earthRotation;
	float sunRotation;

	float pelvisTranslation;
	float pelvisRotation;

	float leftHipTranslation;
	float rightHipTranslation;
	float leftHipRotation;
	float rightHipRotation;
	float leftThighTranslation;
	float rightThighTranslation;
	float leftThighRotation;
	float rightThighRotation;
	float leftTibiaTranslation;
	float rightTibiaTranslation;
	float leftTibiaRotation;
	float rightTibiaRotation;

	float thoraxTranslation;
	float thoraxRotation;

	float leftClavicleTranslation;
	float leftClavicleRotation;
	float rightClavicleTranslation;
	float rightClavicleRotation;

	float leftShoulderTranslation;
	float leftShoulderRotation;
	float rightShoulderTranslation;
	float rightShoulderRotation;

	float leftForearmTranslation;
	float leftForearmRotation;
	float rightForearmTranslation;
	float rightForearmRotation;

	float headTranslation;
	float headRotation;

	float leftEyeTranslation;
	float leftEyeRotation;
	float rightEyeTranslation;
	float rightEyeRotation;

	float thoraxRotationX;
	float thoraxTranslationY;
	float rightFeetRotation;
	float leftFeetRotation;

	Light light[1];
	bool enableLight;

	float idleTime = 0.f;
	bool idleActive = false;

	void AnimateAttack();
	void AnimateIdle();
	void AnimateWave();
	void AnimateSomersault();
	void ResetAllTransforms();
};

#endif