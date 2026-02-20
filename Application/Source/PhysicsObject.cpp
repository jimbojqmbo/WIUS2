#include "PhysicsObject.h"

PhysicsObject::PhysicsObject()
    : sizeX{ 1.f }, sizeY{ 1.f }, sizeZ{ 1.f }, pos{}, prevPos{}, vel{}, accel{},
	mass{ 1.f }, m_totalForces{}, angularVel{}, orientation{}, bounciness{ 1.f }
{
}

PhysicsObject::PhysicsObject(float sizeX, float sizeY, float sizeZ, glm::vec3 pos, float mass, float bounciness)
    : sizeX{ 1.f }, sizeY{ 1.f }, sizeZ{ 1.f }, pos{}, prevPos{}, vel{}, accel{},
	mass{ 1.f }, m_totalForces{}, angularVel{}, orientation{}, bounciness{ 1.f }
{
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->sizeZ = sizeZ;

	this->pos = pos;

	this->mass = mass;
    this->bounciness = bounciness;
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
    if (mass == 0.f)
    {
        m_totalForces = glm::vec3(0.f);
        return;
    }

    // Linear motion
    glm::vec3 finalAccel = accel + m_totalForces / mass;

    vel += finalAccel * dt;

    const float damping = 0.98f;
    vel *= pow(damping, dt * 60.f);

    pos += vel * dt;

    // Angular motion (3D correct)
    glm::quat omega(0.f, angularVel.x, angularVel.y, angularVel.z);
    glm::quat dq = 0.5f * omega * orientation;

    orientation += dq * dt;
    orientation = glm::normalize(orientation);

    m_totalForces = glm::vec3(0.f);
}