#ifndef SCENE_01_H
#define SCENE_01_H

#include "CollisionDetection.h"
#include "PhysicsObject.h"

#include "Scene.h"
#include "Mesh.h"
#include "AltAzCamera.h"
#include "MatrixStack.h"
#include "Light.h"
#include "FPCamera.h"

#include <glm/glm.hpp>
#include <vector>
#include <utility>

struct Player
{
	FPCamera camera;
};

// Simple AABB struct for fence tree
struct AABB {
	glm::vec3 min, max;
	bool intersects(const glm::vec3& point) const {
		return point.x >= min.x && point.x <= max.x &&
			point.y >= min.y && point.y <= max.y &&
			point.z >= min.z && point.z <= max.z;
	}
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
		GEO_DEER,
		GEO_COW,
		GEO_SHEEP,
		GEO_TENT,
		CLOWN,

		GEO_NOTE,
		BUMPERCAR,
		BUMPERCARTENT,
		TALLTREE,
		LOWPOLYBUILDING,
		BLOXBURGTREE,
		CARTOONFENCE,
		CARNIVALTENT,
		ALVINTENT,
		BASKETBALLCOURT,

		GEO_SPARKLING_STAR,
		CUTECHARACTER,

		GEO_PEWPEW,

		GEO_GRASS,
		GEO_ABANDONEDHOUSE,
		GREYGROUND,
		SOLIDGREENGRASS,
		GRASSCLUMP,

		GEO_FLASHLIGHT,
		CONCERTSTAGE,

		GEO_EYEBALL,
		GEO_EYEBALL_MTL,
		JEFFREYEPSTEIN,
		FOREST,
		BIRCHTREE,

		EXITBUTTON,
		PAUSEMENU,
		MAINPAUSE,
		PLAYER1INDICATORUI,
		PLAYER2INDICATORUI,
		ENTERBUMPERCARGAMEPROMPT,
		EXITBUMPERCARGAMEPROMPT,
		ENTERBASKETBALLPROMPT,
		ENTERBALLBOUNCERPROMPT,
		ENTERDUCKSHOOTINGPROMPT,

		NPCINTERACTION,
		NPCINTERACTION2,
		NPCINTERACTION3,

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

	bool scene02request = false;
	bool scene03request = false;
	bool scene04request = false;

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

	void UpdateObjectMovement(double dt);

	void RenderPathway();
	void RenderGamePathways();
	void RenderBirchTrees();
	void RenderGrassClump();

	glm::vec3 change;

	// Mouse control
	float mouseSensitivity = 0.1f;
	bool firstMouse = true;
	double lastMouseX = 400.0;  // Center of 800x600 window
	double lastMouseY = 300.0;

	glm::vec3 objectPos;
	float moveSpeed = 15;

	Player player1;
	Player player2;

	float fps = 0;

	bool BumperCarGameEntered = false;
	bool EnterBumperCarGamePrompt = false;
	bool ExitBumperCarGamePrompt = false;
	bool enterCar1Check = false;
	bool enterCar2Check = false;

	bool BasketballGameEntered = false;
	bool EnterBasketballGamePrompt = false;

	bool BallBouncerGameEntered = false;
	bool EnterBallBouncerGamePrompt = false;

	bool DuckShootingGameEntered = false;
	bool EnterDuckShootingGamePrompt = false;

	bool concertEntered = false;

	bool isNPCinteracted = false;
	bool NPCinteraction = false;
	bool NPCinteraction2 = false;
	bool NPCinteraction3 = false;

	// Physics / bumper-car properties
	glm::vec3 cameraVelocity1 = glm::vec3(0.0f);
	glm::vec3 cameraVelocity2 = glm::vec3(0.0f);

	float cameraMass = 1;      // mass for each bumper car (equal for now)
	float restitution = 2;     // bounce: 0 = inelastic, 1 = perfectly elastic
	float linearDamping = 2;   // damping per second (friction)
	float maxSpeed = 25;       // limit speed from exploding
	float driveAcceleration = 240; // acceleration (units/s^2) from input
	float cameraRadius = 4.5f;    // collision radius per camera

	bool pausemenu = false;
	bool isGameRunning = true;

	bool player1InCar = false;
	bool player2InCar = false;

	// Invisible fence collision zones (AABB min/max)
	// Each entry: first = min, second = max
	std::vector<std::pair<glm::vec3, glm::vec3>> fenceZones;

	std::vector<std::pair<glm::vec3, glm::vec3>> concertStageZone;

	std::vector<std::pair<glm::vec3, glm::vec3>> basketballBuildingZone;

	// Helper to query fence collision
	bool IsInsideFence(const glm::vec3& p) const;

	// Bumper-car world positions (decoupled from camera)
	glm::vec3 bumperCarPos1 = glm::vec3(-11.0f, 0.0f, -90.0f);
	glm::vec3 bumperCarPos2 = glm::vec3(-11.0f, 0.0f, -60.0f);

	float carYaw = glm::radians(0.0f);   // Player 1 car yaw (in radians)
	float carYaw2 = glm::radians(0.0f);  // Player 2 car yaw (in radians)
		
	float gravity = -30.0f;              // units / s^2 (negative)
	float jumpSpeed = 10.0f;             // initial jump velocity
	float cameraVerticalVel1 = 0.0f;     // current vertical velocity for player1
	float cameraVerticalVel2 = 0.0f;     // current vertical velocity for player2
	bool isJumping1 = false;             // whether player1 is mid-jump
	bool isJumping2 = false;;

	// AABB tree for optimized fence collision queries
	std::vector<AABB> fenceTree;

	void UpdateIntro(double dt);
	bool introActive = true;
	bool introReturning = false;

	float introTimer = 0.0f;
	float introDuration = 12.0f;      // seconds to move forward
	float introReturnDuration = 2.0f; // fast return

	glm::vec3 introStartPos = glm::vec3(-67.f, 3.3f, 63.94f);
	glm::vec3 introEndPos = glm::vec3(90.f, 3.3f, 10.f);
};

#endif