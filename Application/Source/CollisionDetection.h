#pragma once

#include <glm\glm.hpp>
#include "PhysicsObject.h"

struct CollisionData
{
	float penetration{};
	glm::vec3 collisionNormal; //collision normal is relative to pObj1
	glm::vec3 contactPoint;
	PhysicsObject* pObj1;
	PhysicsObject* pObj2;
};

//global collision detection functions
bool OverlapCircle2Circle(const glm::vec3& pos1, float r1, const glm::vec3& pos2, float r2);
bool OverlapCircle2Circle(PhysicsObject& circle1, float r1, PhysicsObject& circle2, float r2, CollisionData& cd);
bool OverlapAABB2AABB(const glm::vec3& min1, const glm::vec3& max1,
					  const glm::vec3& min2, const glm::vec3& max2);
bool OverlapAABB2AABB(PhysicsObject& box1, float w1, float h1,
					  PhysicsObject& box2, float w2, float h2, CollisionData& cd);
bool OverlapCircle2Line(const glm::vec3& circlePos, float radius,
						const glm::vec3& lineStart, 
						const glm::vec3& lineEnd);
bool OverlapCircle2OBB(PhysicsObject& circle, float radius, PhysicsObject& box, float w, float h,float w2, CollisionData& cd);

// 3D collision detection functions
bool OverlapSphere2AABB(PhysicsObject& sphereObj, PhysicsObject& boxObj, CollisionData& cd);

// collision resolution function
void ResolveCollision(CollisionData& cd);
void ResolveCircle2StaticLine(PhysicsObject& ball, float radius, const glm::vec3& lineStart, const glm::vec3& lineEnd);