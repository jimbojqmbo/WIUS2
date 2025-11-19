#ifndef SCENE_2_H
#define SCENE_2_H

#include "Scene.h"
#include "Mesh.h"

#include "AltAzCamera.h"
#include "MatrixStack.h"

class Scene2 : public Scene
{
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_SPHERE_ORANGE,
		GEO_SPHERE_BLUE,
		GEO_SPHERE_GREY,
		NUM_GEOMETRY,
	};

	enum UNIFORM_TYPE
	{
		U_MVP = 0,
		U_TOTAL,
	};

	enum ANIMATION
	{
		ANIM_MOON,
		ANIM_EARTH,
		ANIM_SUN,
		NUM_ANIM
	};
	
	// Keep track of current animation
	ANIMATION currAnim;

	Scene2();
	~Scene2();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	void HandleKeyPress();

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
};

#endif