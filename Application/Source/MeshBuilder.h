#ifndef MESH_BUILDER_H
#define MESH_BUILDER_H

#include "Mesh.h"
#include "Vertex.h"

/******************************************************************************/
/*!
		Class MeshBuilder:
\brief	Provides methods to generate mesh of different shapes
*/
/******************************************************************************/
class MeshBuilder
{
public:
	static Mesh* GenerateAxes(const std::string &meshName, float lengthX, float lengthY, float lengthZ);
	static Mesh* GenerateQuad(const std::string& meshName, glm::vec3 color, float length);
	static Mesh* GenerateSphere(const std::string& meshName,
		glm::vec3 color,
		float radius = 1.f,
		int numSlice = 360,
		int numStack = 360);

	static Mesh* GenerateTorus(const std::string& meshName,
		glm::vec3 color,
		float innerR = 1.f,
		float outerR = 1.f,
		int numSlice = 360,
		int numStack = 360);

	static Mesh* GenerateCylinder(const std::string& meshName,
		glm::vec3 color,
		unsigned numSlice = 360,
		float radius = 1.f,
		float height = 2.f);
};

#endif