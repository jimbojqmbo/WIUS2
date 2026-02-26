#ifndef SCENE_02_H
#define SCENE_02_H

#include "CollisionDetection.h"

#include "Scene.h"
#include "Mesh.h"
#include "AltAzCamera.h"
#include "MatrixStack.h"
#include "Light.h"
#include "FPCamera.h"
#include "PhysicsObject.h"
#include "DuckTarget.h"
#include <vector>

class Scene02 : public Scene
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
		BLACK,

		SRANK,
		ARANK,
		BRANK,
		CRANK,
		DRANK,

		GEO_WALL,

		GEO_GRASS,

		GEO_DUCKTARGET,
		GEO_PAPER,

		GEO_FENCE1,
		GEO_FENCE2,
		GEO_FENCE3,
		GEO_FENCE4,
		GEO_FENCE5,

		GEO_TENT,
		GEO_BARREL,
		GEO_CRATE,

		GEO_CRATES1,
		GEO_CRATES2,
		GEO_CRATES3,

		GEO_CRATES4,
		GEO_CRATES5,

		GEO_BARRELS1,
		GEO_BARRELS2,

		GEO_BARRELS3,

		GEO_BLASTER,

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

		// LIGHT 0
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

		// LIGHT 1
		U_LIGHT1_TYPE,
		U_LIGHT1_POSITION,
		U_LIGHT1_COLOR,
		U_LIGHT1_POWER,
		U_LIGHT1_KC,
		U_LIGHT1_KL,
		U_LIGHT1_KQ,
		U_LIGHT1_SPOTDIRECTION,
		U_LIGHT1_COSCUTOFF,
		U_LIGHT1_COSINNER,
		U_LIGHT1_EXPONENT,

		// LIGHT 2
		U_LIGHT2_TYPE,
		U_LIGHT2_POSITION,
		U_LIGHT2_COLOR,
		U_LIGHT2_POWER,
		U_LIGHT2_KC,
		U_LIGHT2_KL,
		U_LIGHT2_KQ,
		U_LIGHT2_SPOTDIRECTION,
		U_LIGHT2_COSCUTOFF,
		U_LIGHT2_COSINNER,
		U_LIGHT2_EXPONENT,

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

	Scene02();
	~Scene02();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	bool startingSceneRequest = false;

private:
	void HandleKeyPress(double dt);
	void RenderMesh(Mesh* mesh, bool enableLight);

	void RenderSkybox();

	void RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey, float rotation);

	void HandleMouseInput(double dt);

	void RenderText(Mesh* mesh, std::string text, glm::vec3 color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y);

	void SpawnTarget(glm::vec3 startingPosition, glm::vec3 endingPosition, glm::vec3 size, float speed, int repeats, int value);

	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];

	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	//AltAzCamera camera;
	FPCamera camera;
	int projType = 1; // fix to 0 for orthographic, 1 for projection

	MatrixStack modelStack, viewStack, projectionStack;

	static const int NUM_LIGHTS = 3;
	Light light[NUM_LIGHTS];
	bool enableLight;

	glm::vec3 change;

	// Mouse control
	float mouseSensitivity = 0.1f;
	bool firstMouse = true;
	double lastMouseX = 400.0;  // Center of 800x600 window
	double lastMouseY = 300.0;

	float moveSpeed = 5.0f;

	// Objects
	std::vector<PhysicsObject> projectiles;
	std::vector<PhysicsObject> walls;
	std::vector<DuckTarget*> targets;

	bool enableHitbox;
	float fps;
	bool wasMousePressed;

	bool wasKeyPressed;

	bool blasterAnimating;
	bool blasterMovingUp;
	float blasterAngle;

	bool gameEnded;

	int score;
	int shotsFired;
	int validTargets;

	glm::vec3 targetHitboxSize;
	glm::vec3 targetSize;

	double totalElapsedTime;
	std::string elapsedTimeText;
	
	PhysicsObject playerHitbox;

	PhysicsObject trigger1;
	PhysicsObject trigger2;
	PhysicsObject trigger3;

	PhysicsObject section1Barrier;
	PhysicsObject section2Barrier;

	bool trigger1Activated;
	bool section1Start;
	bool section1End;
	float trigger1Timer;
	bool section1SpawnEnd;

	bool trigger2Activated;
	bool section2Start;
	bool section2End;
	float trigger2Timer;
	bool section2SpawnEnd;
};

#endif