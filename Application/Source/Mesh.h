#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include "Material.h"
#include <GL/glew.h>

/******************************************************************************/
/*!
		Class Mesh:
\brief	To store VBO (vertex & color buffer) and IBO (index buffer)
*/
/******************************************************************************/
class Mesh
{
public:
	enum DRAW_MODE
	{
		DRAW_TRIANGLES, //default mode
		DRAW_TRIANGLE_STRIP,
		DRAW_LINES,
		DRAW_MODE_LAST,
	};

	GLuint instanceBuffer;
	unsigned instanceCount;

	Mesh(const std::string& meshName);
	~Mesh();

	void Render();

	const std::string name;
	DRAW_MODE mode;
	unsigned vertexBuffer;
	unsigned indexBuffer;
	unsigned indexSize;

	Material material;
	unsigned textureID;

	static void SetMaterialLoc(unsigned kA, unsigned kD, unsigned kS, unsigned nS);
	// Step 11 - Add the following variables
	std::vector<Material> materials;
	static unsigned locationKa;
	static unsigned locationKd;
	static unsigned locationKs;
	static unsigned locationNs;

	void Render(unsigned offset, unsigned count);

	std::vector<glm::vec3> vertices;

	void ComputeAABB(); // compute local-space AABB from vertices
	glm::vec3 aabbMinLocal; // local-space AABB min
	glm::vec3 aabbMaxLocal; // local-space AABB max

	// Returns {worldMin, worldMax} after transforming local AABB by model matrix
	std::pair<glm::vec3, glm::vec3> GetAABBWorld(const glm::mat4& model) const;
};

#endif