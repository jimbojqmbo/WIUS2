#ifndef SCENE_04_H
#define SCENE_04_H
//kyler
#include "CollisionDetection.h"

#include "Scene.h"
#include "Mesh.h"
#include "AltAzCamera.h"
#include "MatrixStack.h"
#include "Light.h"
#include "FPCamera.h"

class Scene04 : public Scene
{
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_QUAD,
		GEO_SPHERE,
		GEO_TOP,
		GEO_BOTTOM,
		GEO_LEFT,
		GEO_RIGHT,
		GEO_FRONT,
		GEO_BACK,
		GEO_GUI,
		GEO_CYLINDER,
		GEO_TEXT,
		GEO_SHADOW,

		GEO_NOTE,

		GEO_SPARKLING_STAR,

		GEO_PEWPEW,

		GEO_GRASS,

		GEO_PINETREE,
		GEO_ABANDONEDHOUSE,

		GEO_FLASHLIGHT,

		GEO_EYEBALL,
		GEO_EYEBALL_MTL,

		NUM_GEOMETRY,
	};

	enum UNIFORM_TYPE
	{
		U_MVP = 0,
		U_MODELVIEW,
		U_MODELVIEW_INVERSE_TRANSPOSE,
		U_MATERIAL_AMBIENT,
		U_MATERIAL_DIFFUSE,
		U_MATERIAL_SPECULAR,
		U_MATERIAL_SHININESS,
		U_LIGHT0_TYPE,
		U_LIGHT0_POSITION,
		U_LIGHT0_COLOR,
		U_LIGHT0_POWER,
		U_LIGHT0_KC,
		U_LIGHT0_KL,
		U_LIGHT0_KQ,
		U_LIGHT0_SPOTDIRECTION,
		U_LIGHT0_COSCUTOFF,
		U_LIGHT0_COSINNER,
		U_LIGHT0_EXPONENT,
		U_NUMLIGHTS,
		U_COLOR_TEXTURE_ENABLED,
		U_COLOR_TEXTURE,
		U_LIGHTENABLED,
		U_TEXT_ENABLED,
		U_TEXT_COLOR,

		// Fog uniforms
		U_FOG_ENABLED,
		U_FOG_START,
		U_FOG_END,
		U_FOG_COLOR,

		U_TOTAL,
	};

	Scene04();
	~Scene04();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	void HandleKeyPress(double dt);
	void RenderMesh(Mesh* mesh, bool enableLight);

	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];

	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	//AltAzCamera camera;
	FPCamera camera;
	int projType = 1; // fix to 0 for orthographic, 1 for projection

	MatrixStack modelStack, viewStack, projectionStack;

	static const int NUM_LIGHTS = 1;
	Light light[NUM_LIGHTS];
	bool enableLight;

	void RenderSkybox();

	void RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey);

	void HandleMouseInput();

	void RenderText(Mesh* mesh, std::string text, glm::vec3 color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y);

	glm::vec3 change;

	// Mouse control
	float mouseSensitivity = 0.1f;
	bool firstMouse = true;
	double lastMouseX = 400.0;  // Center of 800x600 window
	double lastMouseY = 300.0;

	float moveSpeed = 5.0f;
	//additional
	// idk
	static const int  ball_num = 10;
	//objects
	PhysicsObject ball[ball_num];
	PhysicsObject floor;
	PhysicsObject ball_slide;
	//varibles
	// game scene
	float gravity = 10;
	//size
	float ball_diameter = 2;
	float floor_space = 10;
	//functions

	bool OverlapCircle2CYLINDER(const glm::vec3& pos1, float r1, const glm::vec3& pos2, float width,float height);
	void balls_update(double dt);

};

#endif