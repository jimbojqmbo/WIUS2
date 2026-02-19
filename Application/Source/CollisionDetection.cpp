#include "CollisionDetection.h"
#include <cmath>
#include <algorithm>

// Math Functions
float LengthSquared(const glm::vec3& vector)
{
	return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

bool IsZero(const glm::vec3 vector)
{
	return fabs(vector.x) < 1e-8f && fabs(vector.y) < 1e-8f && fabs(vector.z) < 1e-8f;
}

float Length(const glm::vec3 vector)
{
	return std::sqrt(LengthSquared(vector));
}

glm::vec3 Normalize(const glm::vec3& vector)
{
	float len = Length(vector);
	if (len > 1e-8f)
		return vector / len;
	else
		return glm::vec3(0.f); // can't normalize zero vector
}

float Dot(glm::vec3 vector1, glm::vec3 vector2)
{
	return vector1.x * vector2.x + vector1.y * vector2.x + vector1.z * vector2.z;
}

// Positional circle-vs-circle (already present)
bool OverlapCircle2Circle(const glm::vec3& pos1, float r1, const glm::vec3& pos2, float r2)
{
	float lengthSq = LengthSquared(pos1 - pos2);
	return lengthSq <= (r1 + r2) * (r1 + r2);
}

// PhysicsObject-based circle-vs-circle that fills CollisionData
bool OverlapCircle2Circle(PhysicsObject& circle1, float r1, PhysicsObject& circle2, float r2, CollisionData& cd)
{
	glm::vec3 n = circle2.pos - circle1.pos;
	float dist2 = LengthSquared(n);
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

// Circle vs line segment (positional) - already implemented but repeated here for completeness
bool OverlapCircle2Line(const glm::vec3& circlePos, float radius,
	const glm::vec3& lineStart,
	const glm::vec3& lineEnd)
{
	glm::vec3 lineTangent = lineEnd - lineStart;
	if (IsZero(lineTangent)) return false;  // not a valid segment

	float lineLength = Length(lineTangent);
	Normalize(lineTangent);

	// normal to the line
	glm::vec3 lineNormal{ -lineTangent.y, lineTangent.x, 0.f };

	// distance from circle center to infinite line
	float distToLine = Dot(circlePos - lineStart, lineNormal);

	if (fabs(distToLine) > radius)
		return false;

	// projection along line tangent
	glm::vec3 lineVec = circlePos - lineStart;
	float projectedDist = Dot(lineVec, lineTangent);

	// check against segment endpoints if outside
	if (projectedDist > lineLength)
	{
		glm::vec3 diff = circlePos - lineEnd;
		float distSq = LengthSquared(diff);
		if (distSq > radius * radius)
			return false;
	}
	else if (projectedDist < 0)
	{
		glm::vec3 diff = circlePos - lineStart;
		float distSq = LengthSquared(diff);
		if (distSq > radius * radius)
			return false;
	}

	return true;
}

// Circle vs OBB (implement as circle vs AABB aligned with axes centered on box.pos using half-extents w,h)
bool OverlapCircle2OBB(PhysicsObject& circle, float radius, PhysicsObject& box, float w, float h, CollisionData& cd)
{
	glm::vec3 min = box.pos - glm::vec3(w, h, 0.f);
	glm::vec3 max = box.pos + glm::vec3(w, h, 0.f);

	// closest point on AABB to circle center
	float cx = std::max(min.x, std::min(circle.pos.x, max.x));
	float cy = std::max(min.y, std::min(circle.pos.y, max.y));
	glm::vec3 closest(cx, cy, 0.f);

	glm::vec3 diff = circle.pos - closest;
	float dist2 = LengthSquared(diff);

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

// General collision resolver placeholder - scenes may use CollisionData to resolve
void ResolveCollision(CollisionData& cd)
{
	// Leave resolution to scene-specific logic (needs restitution, friction etc.)
	(void)cd;
}

// Resolve circle vs static line using PhysicsObject interface
void ResolveCircle2StaticLine(PhysicsObject& ball, float radius, const glm::vec3& lineStart, const glm::vec3& lineEnd)
{
	glm::vec3 lineTangent = lineEnd - lineStart;
	float lineLength = Length(lineTangent);
	if (lineLength <= 1e-6f) return;
	Normalize(lineTangent);

	glm::vec3 lineVec = ball.pos - lineStart;
	float projectedDist = Dot(lineVec,lineTangent);

	glm::vec3 normal;
	float penetrationDist = 0.f;

	// ball beyond line end
	if (projectedDist > lineLength)
	{
		glm::vec3 closest = lineEnd;
		glm::vec3 fromClosest = ball.pos - closest;
		float dist = Length(fromClosest);
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = glm::vec3(-lineTangent.y, lineTangent.x, 0.f);
			Normalize(normal);
			penetrationDist = radius;
		}
	}
	// ball before line start
	else if (projectedDist < 0.f)
	{
		glm::vec3 closest = lineStart;
		glm::vec3 fromClosest = ball.pos - closest;
		float dist = Length(fromClosest);
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = glm::vec3(-lineTangent.y, lineTangent.x, 0.f);
			Normalize(normal);
			penetrationDist = radius;
		}
	}
	// ball projects onto segment
	else
	{
		glm::vec3 closest = lineStart + lineTangent * projectedDist;
		glm::vec3 fromClosest = ball.pos - closest;
		float dist = Length(fromClosest);
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = glm::vec3(-lineTangent.y, lineTangent.x, 0.f);
			Normalize(normal);
			penetrationDist = radius;
		}
	}

	// resolve penetration
	if (penetrationDist > 0.f)
	{
		ball.pos += normal * penetrationDist;
		// remove normal component of velocity
		float vn = Dot(ball.vel,normal);
		ball.vel -= normal * vn;
	}
}