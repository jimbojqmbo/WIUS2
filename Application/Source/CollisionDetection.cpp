#include "CollisionDetection.h"
#include <cmath>
#include <algorithm>

// Positional circle-vs-circle (already present)
bool OverlapCircle2Circle(const glm::vec3& pos1, float r1, const glm::vec3& pos2, float r2)
{
	glm::vec3 d = pos1 - pos2;
	float lengthSq = glm::dot(d, d);
	float rsum = r1 + r2;
	return lengthSq <= rsum * rsum;
}

// PhysicsObject-based circle-vs-circle that fills CollisionData
bool OverlapCircle2Circle(PhysicsObject& circle1, float r1, PhysicsObject& circle2, float r2, CollisionData& cd)
{
	glm::vec3 n = circle2.pos - circle1.pos;
	float dist2 = glm::dot(n, n);
	float rsum = r1 + r2;

	// no overlap
	if (dist2 > rsum * rsum)
		return false;

	cd.pObj1 = &circle1;
	cd.pObj2 = &circle2;

	if (dist2 <= 1e-8f)
	{
		// coincident centers
		cd.collisionNormal = glm::vec3(1.f, 0.f, 0.f);
		cd.penetration = rsum;
		cd.contactPoint = circle1.pos;
	}
	else
	{
		float dist = std::sqrt(dist2);
		cd.collisionNormal = n / dist; // points from circle1 -> circle2
		cd.penetration = rsum - dist;
		// approximate contact point halfway between the two surface points
		cd.contactPoint = circle1.pos + cd.collisionNormal * (r1 - cd.penetration * 0.5f);
	}

	return true;
}

// AABB min/max check (positional)
bool OverlapAABB2AABB(const glm::vec3& min1, const glm::vec3& max1,
	const glm::vec3& min2, const glm::vec3& max2)
{
	// Correct AABB overlap test
	return !(max1.x < min2.x || min1.x > max2.x || max1.y < min2.y || min1.y > max2.y);
}

// PhysicsObject-based AABB vs AABB (w,h are half-extents). Fills CollisionData.
bool OverlapAABB2AABB(PhysicsObject& box1, float w1, float h1,
	PhysicsObject& box2, float w2, float h2, CollisionData& cd)
{
	glm::vec3 min1 = box1.pos - glm::vec3(w1, h1, 0.f);
	glm::vec3 max1 = box1.pos + glm::vec3(w1, h1, 0.f);
	glm::vec3 min2 = box2.pos - glm::vec3(w2, h2, 0.f);
	glm::vec3 max2 = box2.pos + glm::vec3(w2, h2, 0.f);

	if (!OverlapAABB2AABB(min1, max1, min2, max2))
		return false;

	// compute overlap on each axis
	float overlapX = std::min(max1.x, max2.x) - std::max(min1.x, min2.x);
	float overlapY = std::min(max1.y, max2.y) - std::max(min1.y, min2.y);

	// choose smallest penetration axis
	if (overlapX < overlapY)
	{
		// normal points from box1 -> box2 along x
		cd.collisionNormal = (box1.pos.x < box2.pos.x) ? glm::vec3(-1.f, 0.f, 0.f) : glm::vec3(1.f, 0.f, 0.f);
		cd.penetration = overlapX;
	}
	else
	{
		cd.collisionNormal = (box1.pos.y < box2.pos.y) ? glm::vec3(0.f, -1.f, 0.f) : glm::vec3(0.f, 1.f, 0.f);
		cd.penetration = overlapY;
	}

	cd.pObj1 = &box1;
	cd.pObj2 = &box2;

	// approximate contact point (center of overlap region)
	cd.contactPoint.x = (std::max(min1.x, min2.x) + std::min(max1.x, max2.x)) * 0.5f;
	cd.contactPoint.y = (std::max(min1.y, min2.y) + std::min(max1.y, max2.y)) * 0.5f;
	cd.contactPoint.z = 0.f;

	return true;
}

// Circle vs line segment (positional)
bool OverlapCircle2Line(const glm::vec3& circlePos, float radius,
	const glm::vec3& lineStart,
	const glm::vec3& lineEnd)
{
	glm::vec3 lineTangent = lineEnd - lineStart;
	float lineLength = glm::length(lineTangent);
	if (lineLength <= 1e-8f) return false;  // not a valid segment

	lineTangent = glm::normalize(lineTangent);

	// normal to the line
	glm::vec3 lineNormal{ -lineTangent.y, lineTangent.x, 0.f };

	// distance from circle center to infinite line
	float distToLine = glm::dot(circlePos - lineStart, lineNormal);

	if (std::fabs(distToLine) > radius)
		return false;

	// projection along line tangent
	glm::vec3 lineVec = circlePos - lineStart;
	float projectedDist = glm::dot(lineVec, lineTangent);

	// check against segment endpoints if outside
	if (projectedDist > lineLength)
	{
		glm::vec3 diff = circlePos - lineEnd;
		float distSq = glm::dot(diff, diff);
		if (distSq > radius * radius)
			return false;
	}
	else if (projectedDist < 0)
	{
		glm::vec3 diff = circlePos - lineStart;
		float distSq = glm::dot(diff, diff);
		if (distSq > radius * radius)
			return false;
	}

	return true;
}

// Circle vs OBB (implement as circle vs AABB aligned with axes centered on box.pos using half-extents w,h)
bool OverlapCircle2OBB(PhysicsObject& circle, float radius, PhysicsObject& box, float w, float h,float w2, CollisionData& cd)
{
	glm::vec3 min = box.pos - glm::vec3(w, h, w2);
	glm::vec3 max = box.pos + glm::vec3(w, h, w2);

	// closest point on AABB to circle center
	float cx = std::max(min.x, std::min(circle.pos.x, max.x));
	float cy = std::max(min.y, std::min(circle.pos.y, max.y));
	float cz = std::max(min.z, std::min(circle.pos.z, max.z));
	glm::vec3 closest(cx, cy, cz);

	glm::vec3 diff = circle.pos - closest;
	float dist2 = glm::dot(diff, diff);

	if (dist2 > radius * radius)
		return false;

	cd.pObj1 = &circle;
	cd.pObj2 = &box;
	cd.contactPoint = closest;

	if (dist2 <= 1e-8f)
	{
		cd.collisionNormal = glm::vec3(0.f, 1.f, 0.f);
		cd.penetration = radius;
	}
	else
	{
		float dist = std::sqrt(dist2);
		cd.collisionNormal = diff / dist; // from box -> circle
		cd.penetration = radius - dist;
	}

	return true;
}

bool OverlapSphere2AABB(PhysicsObject& sphereObj, PhysicsObject& boxObj, CollisionData& cd)
{
	glm::vec3 spherePos = sphereObj.pos;
	float radius = sphereObj.sizeX * 0.5f;

	glm::vec3 halfSize(boxObj.sizeX * 0.5f, boxObj.sizeY * 0.5f, boxObj.sizeZ * 0.5f);

	glm::vec3 boxMin = boxObj.pos - halfSize;
	glm::vec3 boxMax = boxObj.pos + halfSize;

	// Prepare to store the closest collision
	float minPenetration = FLT_MAX;
	glm::vec3 bestNormal(0.f);
	glm::vec3 bestContact(0.f);
	bool collided = false;

	// Define each face (normal and plane coordinate)
	struct Face { glm::vec3 normal; glm::vec3 point; };
	Face faces[6] = {
		{ glm::vec3(1, 0, 0), boxMax }, // +X
		{ glm::vec3(-1, 0, 0), boxMin }, // -X
		{ glm::vec3(0, 1, 0), boxMax }, // +Y
		{ glm::vec3(0,-1, 0), boxMin }, // -Y
		{ glm::vec3(0, 0, 1), boxMax }, // +Z
		{ glm::vec3(0, 0,-1), boxMin }  // -Z
	};

	// Check each face
	for (int i = 0; i < 6; ++i)
	{
		Face f = faces[i];

		// Project sphere center onto the face plane (clamp to face bounds)
		glm::vec3 closest = spherePos;
		if (f.normal.x != 0) {
			closest.x = f.point.x;
			closest.y = glm::clamp(spherePos.y, boxMin.y, boxMax.y);
			closest.z = glm::clamp(spherePos.z, boxMin.z, boxMax.z);
		}
		if (f.normal.y != 0) {
			closest.y = f.point.y;
			closest.x = glm::clamp(spherePos.x, boxMin.x, boxMax.x);
			closest.z = glm::clamp(spherePos.z, boxMin.z, boxMax.z);
		}
		if (f.normal.z != 0) {
			closest.z = f.point.z;
			closest.x = glm::clamp(spherePos.x, boxMin.x, boxMax.x);
			closest.y = glm::clamp(spherePos.y, boxMin.y, boxMax.y);
		}

		glm::vec3 diff = spherePos - closest;
		float distSquared = glm::dot(diff, diff);

		if (distSquared <= radius * radius)
		{
			float penetration = radius - sqrt(distSquared);
			if (penetration < minPenetration)
			{
				minPenetration = penetration;
				bestNormal = f.normal;
				bestContact = closest;
				collided = true;
			}
		}
	}

	if (collided)
	{
		cd.pObj1 = &sphereObj;
		cd.pObj2 = &boxObj;
		cd.collisionNormal = bestNormal;
		cd.penetration = minPenetration;
		cd.contactPoint = bestContact;
		return true;
	}

	return false;
}

void ResolveCollision(CollisionData& cd)
{
	PhysicsObject& Obj1 = *cd.pObj1;
	PhysicsObject& Obj2 = *cd.pObj2;

	glm::vec3 n = glm::normalize(cd.collisionNormal);

	float invMass1 = (Obj1.mass == 0.f) ? 0.f : 1.f / Obj1.mass;
	float invMass2 = (Obj2.mass == 0.f) ? 0.f : 1.f / Obj2.mass;
	float totalInvMass = invMass1 + invMass2;
	if (totalInvMass == 0.f) return;

	// --- Immediate positional correction ---
	Obj1.pos += n * cd.penetration; // fully move sphere out of wall

	// --- Compute relative velocity along normal ---
	float velAlongNormal = glm::dot(Obj1.vel - Obj2.vel, n);

	// --- Apply bounciness ---
	float restitution = std::min(Obj1.bounciness, Obj2.bounciness);

	if (velAlongNormal < 0.f) // only if moving into the wall
	{
		float j = -(1.f + restitution) * velAlongNormal / totalInvMass;
		glm::vec3 impulse = j * n;

		Obj1.vel += impulse * invMass1;
		Obj2.vel -= impulse * invMass2; // wall usually has invMass=0
	}

	// --- Friction along tangent ---
	glm::vec3 relativeVel = Obj1.vel - Obj2.vel;
	glm::vec3 tangent = relativeVel - glm::dot(relativeVel, n) * n;
	float lenT = glm::length(tangent);
	if (lenT > 0.001f)
	{
		tangent /= lenT;
		glm::vec3 frictionImpulse = -0.4f * tangent * glm::length(relativeVel); // simple friction
		Obj1.vel += frictionImpulse * invMass1;
		Obj2.vel -= frictionImpulse * invMass2;
	}

	// Clamp very small velocities
	if (glm::length(Obj1.vel) < 0.01f) Obj1.vel = glm::vec3(0.f);
}

// Resolve circle vs static line using PhysicsObject interface
void ResolveCircle2StaticLine(PhysicsObject& ball, float radius, const glm::vec3& lineStart, const glm::vec3& lineEnd)
{
	glm::vec3 lineTangent = lineEnd - lineStart;
	float lineLength = glm::length(lineTangent);
	if (lineLength <= 1e-6f) return;
	lineTangent = glm::normalize(lineTangent);

	glm::vec3 lineVec = ball.pos - lineStart;
	float projectedDist = glm::dot(lineVec, lineTangent);

	glm::vec3 normal;
	float penetrationDist = 0.f;

	// ball beyond line end
	if (projectedDist > lineLength)
	{
		glm::vec3 closest = lineEnd;
		glm::vec3 fromClosest = ball.pos - closest;
		float dist = glm::length(fromClosest);
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = glm::vec3(-lineTangent.y, lineTangent.x, 0.f);
			normal = glm::normalize(normal);
			penetrationDist = radius;
		}
	}
	// ball before line start
	else if (projectedDist < 0.f)
	{
		glm::vec3 closest = lineStart;
		glm::vec3 fromClosest = ball.pos - closest;
		float dist = glm::length(fromClosest);
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = glm::vec3(-lineTangent.y, lineTangent.x, 0.f);
			normal = glm::normalize(normal);
			penetrationDist = radius;
		}
	}
	// ball projects onto segment
	else
	{
		glm::vec3 closest = lineStart + lineTangent * projectedDist;
		glm::vec3 fromClosest = ball.pos - closest;
		float dist = glm::length(fromClosest);
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = glm::vec3(-lineTangent.y, lineTangent.x, 0.f);
			normal = glm::normalize(normal);
			penetrationDist = radius;
		}
	}

	// resolve penetration
	if (penetrationDist > 0.f)
	{
		ball.pos += normal * penetrationDist;
		// remove normal component of velocity
		float vn = glm::dot(ball.vel, normal);
		ball.vel -= normal * vn;
	}
}
bool OverlapCircle2AABB(glm::vec3 circlePos, float radius, glm::vec3 boxPos, glm::vec3 box_daimension)
{

	glm::vec3 boxMin;
	boxMin = glm::vec3(boxPos.x-(box_daimension.x/2), boxPos.y - (box_daimension.y / 2), boxPos.z - (box_daimension.z / 2));
	glm::vec3 boxMax;
	boxMax = glm::vec3(boxPos.x + (box_daimension.x / 2), boxPos.y + (box_daimension.y / 2), boxPos.z + (box_daimension.z / 2));

	int nearx = circlePos.x;
	if (nearx < boxMin.x) {
		nearx = boxMin.x;
	}
	else if (nearx > boxMax.x) {
		nearx = boxMax.x;
	}
	int neary = circlePos.y;
	if (neary < boxMin.y) {
		neary = boxMin.y;
	}
	else if (neary > boxMax.y) {
		neary = boxMax.y;
	}
	int distx = circlePos.x - nearx;
	int disty = circlePos.y - neary;

	return (distx * distx + disty * disty) <= (radius * radius);
}

bool OverlapCircle2AABB(PhysicsObject circle, float radius, PhysicsObject box, glm::vec3 box_daimension,CollisionData cd)
{
	

	glm::vec3 circlePos;
	circlePos = circle.pos;
	glm::vec3 boxPos;
	boxPos = box.pos;
	glm::vec3 min;
	min = glm::vec3(boxPos.x - (box_daimension.x / 2), boxPos.y - (box_daimension.y / 2), boxPos.z - (box_daimension.z / 2));
	glm::vec3 max;
	max = glm::vec3(boxPos.x + (box_daimension.x / 2), boxPos.y + (box_daimension.y / 2), boxPos.z + (box_daimension.z / 2));

	glm::vec3 closestPoint;
	closestPoint.x = glm::clamp(circlePos.x, min.x, max.x);
	closestPoint.y = glm::clamp(circlePos.y, min.y, max.y);
	closestPoint.z = glm::clamp(circlePos.z, min.z, max.z);

	glm::vec3 n = circlePos - closestPoint;
	float dist2 = glm::dot(n, n);

	if (dist2 > radius * radius)
		return false;

	float dist = std::sqrt(dist2);

	cd.pObj1 = &circle;
	cd.pObj2 = &box;
	std::cout << cd.pObj1->mass;
	cd.collisionNormal = (dist > 1e-8f) ? n / dist : glm::vec3(1.f, 0.f, 0.f);
	cd.penetration = radius - dist;
	cd.contactPoint = closestPoint;
	

	return true;

}