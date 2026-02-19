#pragma once

#include <glm\glm.hpp>

class PhysicsObject
{
public:
	PhysicsObject();
	virtual ~PhysicsObject() = default;

	//for linear movement
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 accel;
	float mass;

	//for angular movement
	float angularVel; //degrees per second
	float angleDeg;   //degrees

	//misc
	float bounciness;

	void AddForce(const glm::vec3& force); //add a pushing force through the center of mass
	void AddImpulse(const glm::vec3& impulse); //an impulse results in an immediate change in velocity
	void UpdatePhysics(float dt);

protected:
	glm::vec3 m_totalForces;
};
