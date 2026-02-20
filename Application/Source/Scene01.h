#ifndef SCENE_01_H
#define SCENE_01_H

#include "CollisionDetection.h"

#include "Scene.h"
#include "Mesh.h"
#include "AltAzCamera.h"
#include "MatrixStack.h"
#include "Light.h"
#include "FPCamera.h"

struct Player
{
	FPCamera camera;
};

class Scene01 : public Scene
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
		BUMPERCAR,
		TALLTREE,

		GEO_SPARKLING_STAR,

		GEO_PEWPEW,

		GEO_GRASS,
		GEO_ABANDONEDHOUSE,

		GEO_FLASHLIGHT,

		GEO_EYEBALL,
		GEO_EYEBALL_MTL,
		JEFFREYEPSTEIN,
		FOREST,

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

	Scene01();
	~Scene01();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	void HandleKeyPress1(FPCamera& cam, double dt);
	void HandleKeyPress2(FPCamera& cam, double dt);
	void RenderMesh(Mesh* mesh, bool enableLight);

	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];

	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	//AltAzCamera camera;
	//FPCamera camera;
	FPCamera camera1;
	FPCamera camera2;
	int projType = 1; // fix to 0 for orthographic, 1 for projection

	MatrixStack modelStack, viewStack, projectionStack;

	static const int NUM_LIGHTS = 1;
	Light light[NUM_LIGHTS];
	bool enableLight;

	void RenderSkybox();
	void RenderSceneFromCamera(FPCamera& cam);

	void RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey);

	void HandleMouseInput(FPCamera& cam);

	void RenderText(Mesh* mesh, std::string text, glm::vec3 color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y);

	glm::vec3 change;

	// Mouse control
	float mouseSensitivity = 0.1f;
	bool firstMouse = true;
	double lastMouseX = 400.0;  // Center of 800x600 window
	double lastMouseY = 300.0;

	float moveSpeed = 15;

	Player player1;
	Player player2;

	float fps = 0;

	// Physics / bumper-car properties
	glm::vec3 cameraVelocity1 = glm::vec3(0.0f);
	glm::vec3 cameraVelocity2 = glm::vec3(0.0f);

	float cameraMass = 1;      // mass for each bumper car (equal for now)
	float restitution = 4;     // bounce: 0 = inelastic, 1 = perfectly elastic
	float linearDamping = 2;   // damping per second (friction)
	float maxSpeed = 25;       // limit speed from exploding
	float driveAcceleration = 240; // acceleration (units/s^2) from input
	float cameraRadius = 3.5;    // collision radius per camera

	// Resolve collision by applying an impulse to velocities and a small positional correction (XZ-plane)
	void ResolveCameraCollisionsWithBounce(FPCamera& a, glm::vec3& velA, FPCamera& b, glm::vec3& velB, double dt);
	
};

#endif