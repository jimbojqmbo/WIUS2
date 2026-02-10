#ifndef SCENE_TEXT_H
#define SCENE_TEXT_H

#include "Scene.h"
#include "Mesh.h"
#include "AltAzCamera.h"
#include "MatrixStack.h"
#include "Light.h"
#include "FPCamera.h"

struct CollisionData
{
	glm::vec3 collisionNormal;
	float penetration;
	glm::vec3 contactPoint;
};

class SceneText : public Scene
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

	SceneText();
	~SceneText();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	void HandleKeyPress(double dt);
	void RenderMesh(Mesh* mesh, bool enableLight);

	// Tree collision helper (AABB grid matches Render() layout)
	void HandleTreeCollisions();
	static constexpr float kTreeHalfWidth = 0.5f;   // world-space half-width for a tree (after scale)
	static constexpr float kTreeHalfHeight = 1.0f;  // world-space half-height for a tree (after scale)
	static constexpr float kTreeStart = -250.f;
	static constexpr float kTreeEnd = 250.f;
	static constexpr float kTreeStep = 21.f;

	// Collision helpers for GEO_COBBLESTONE
	void UpdateCobblestoneAABB(); // compute world-space AABB for the cobblestone quad
	bool IsPointInsideAABB(const glm::vec3& point, const glm::vec3& min, const glm::vec3& max) const;

	// AABB overlap helper (uses 2D X/Y extents). Returns true and fills cd if overlap exists.
	static bool OverlapAABB2AABB(const glm::vec3& pos1, float w1, float h1,
		const glm::vec3& pos2, float w2, float h2, CollisionData& cd);

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
	void RenderFlashlight();

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

	// Fog control
	bool fogEnabled = true;
	float fogStart = 5.0f;
	float fogEnd = 10.0f;
	glm::vec3 fogColor = glm::vec3(0.0f, 0.0f, 0.0f); // fade to black by default

	bool showDark = true; 
	bool flashlightOn = false;
	glm::vec3 flashlightWorldPos = glm::vec3(15.f, 2.f, 38.f);

	bool flashlightObjective = true;
	bool runObjective = false;

	// In your header file
	bool cutsceneActivation = false;
	bool isShadowSpawned = false;
	float shadowY = -10.0f; // Start below the ground
	float shadowTargetY = 1.0f;  // Your desired final height
	float shadowRiseSpeed = 4.0f; // Units per second

	float moveSpeed = 5.0f;

	// Add these to the SceneText class (e.g., in the private or public section)
	float bobTime = 0.0f;       // Accumulates time for bobbing oscillation
	float bobSpeed = 25.0f;      // Speed of bobbing (higher = faster)
	float bobAmount = 0.1f;     // Amplitude of bobbing (vertical offset in units)

	// Add these to the SceneText class (e.g., in the private section)
	bool shadowFollowStarted = false;  // Flag to track if following has begun
	float shadowFollowTimer = 0.5f;   // Timer for the delay before following starts

	// Add these to the SceneText class (e.g., in the private section)
	glm::vec3 shadowCurrentPos;     // Current position of the shadow for lerping
	float shadowLerpSpeed = 0.01f; // Speed of pursuit (0.0f = no movement, 1.0f = instant; lower = slower/laggy pursuit)

	// Add these to the SceneText class (e.g., in the private section)
	bool shadowStillActive = false;  // Flag for stillness period
	float shadowStillTimer = 0.0f;  // Timer for stillness duration

	bool isPlayerDead = false;

	bool isEndActivated = false;

	bool startOfGame = true;
	float StarttextTimer = 7.0f;
	float ChasetextTimer = 6.0f;
	float flashlighttextTimer = 5.0f;
};

#endif