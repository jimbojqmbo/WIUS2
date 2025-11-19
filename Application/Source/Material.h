#ifndef MATERIAL_H
#define MATERIAL_H

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

// Define the material in C++
struct Material
{
	glm::vec3 kAmbient;
	glm::vec3 kDiffuse;
	glm::vec3 kSpecular;
	float kShininess;
	Material() :
		kAmbient(0.0f, 0.0f, 0.0f), kDiffuse(0.0f, 0.0f, 0.0f), kSpecular(0.0f, 0.0f, 0.0f),
		kShininess(1.f)
	{
	}
};

#endif