#ifndef LIGHT_H
#define LIGHT_H

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

struct Light
{
	glm::vec3 position; // Define the light location
	glm::vec3 color; // Define the light color
	float power; // Define the light power
	float kC, kL, kQ; // Variables to calculate attenuation

	Light() : position(0, 20, 0), color(1, 1, 1), power(1),
		kC(1), kL(0.1f), kQ(0.001f)
	{
	}
};

#endif