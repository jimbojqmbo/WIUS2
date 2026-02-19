#include "PhysicsObject.h"

PhysicsObject::PhysicsObject()
	: pos{}, vel{}, accel{}, mass{ 1.f }, m_totalForces{},
	  angularVel{}, angleDeg{}, bounciness{1.f}
{
}

void PhysicsObject::AddForce(const glm::vec3& force)
{
	m_totalForces += force;
}

void PhysicsObject::AddImpulse(const glm::vec3& impulse)
{
	//TODO
	if (mass == 0)
		return;

	vel += impulse * (1.0f / mass);
}

void PhysicsObject::UpdatePhysics(float dt)
{
	//TODO
	glm::vec3 finalAccel = accel;
	if (mass > 0)
	{
		finalAccel += m_totalForces * (1.f / mass);
	}

	vel += finalAccel * dt;
	pos += vel * dt;

	angleDeg += angularVel * dt;

	m_totalForces = glm::vec3(0.f);
}
