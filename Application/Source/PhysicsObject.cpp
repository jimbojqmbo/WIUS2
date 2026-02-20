#include "PhysicsObject.h"

PhysicsObject::PhysicsObject()
	: sizeX{1.f}, sizeY{1.f}, sizeZ{1.f}, pos{}, vel{}, accel{}, 
	mass{ 1.f }, m_totalForces{}, angularVel{}, angleDeg{}, bounciness{ 1.f }
{
}

PhysicsObject::PhysicsObject(float sizeX, float sizeY, float sizeZ, glm::vec3 pos)
	: sizeX{ 1.f }, sizeY{ 1.f }, sizeZ{ 1.f }, pos{}, vel{}, accel{},
	mass{ 1.f }, m_totalForces{}, angularVel{}, angleDeg{}, bounciness{ 1.f }
{
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->sizeZ = sizeZ;

	this->pos = pos;
}

void PhysicsObject::AddForce(const glm::vec3& force)
{
	m_totalForces += force;
}

void PhysicsObject::AddImpulse(const glm::vec3& impulse)
{
	// immediate velocity change
	if (mass == 0.0f)
		return;

	vel += impulse * (1.0f / mass);
}

void PhysicsObject::UpdatePhysics(float dt)
{
	glm::vec3 finalAccel = accel;
	if (mass > 0.0f)
	{
		finalAccel += m_totalForces * (1.f / mass);
	}

	vel += finalAccel * dt;
	pos += vel * dt;

	angleDeg += angularVel * dt;

	m_totalForces = glm::vec3(0.f);
}
