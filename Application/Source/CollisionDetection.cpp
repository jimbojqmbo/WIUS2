#include "CollisionDetection.h"
#include <cmath>
#include <algorithm>

// Positional circle-vs-circle (already present)
bool OverlapCircle2Circle(const Vector3& pos1, float r1, const Vector3& pos2, float r2)
{
	float lengthSq = (pos1 - pos2).LengthSquared();
	return lengthSq <= (r1 + r2) * (r1 + r2);
}

// PhysicsObject-based circle-vs-circle that fills CollisionData
bool OverlapCircle2Circle(PhysicsObject& circle1, float r1, PhysicsObject& circle2, float r2, CollisionData& cd)
{
	Vector3 n = circle2.pos - circle1.pos;
	float dist2 = n.LengthSquared();
	float rsum = r1 + r2;

	// no overlap
	if (dist2 > rsum * rsum)
		return false;

	cd.pObj1 = &circle1;
	cd.pObj2 = &circle2;

	if (dist2 <= 1e-8f)
	{
		// coincident centers
		cd.collisionNormal = Vector3(1.f, 0.f, 0.f);
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
bool OverlapAABB2AABB(const Vector3& min1, const Vector3& max1,
	const Vector3& min2, const Vector3& max2)
{
	// Correct AABB overlap test
	return !(max1.x < min2.x || min1.x > max2.x || max1.y < min2.y || min1.y > max2.y);
}

// PhysicsObject-based AABB vs AABB (w,h are half-extents). Fills CollisionData.
bool OverlapAABB2AABB(PhysicsObject& box1, float w1, float h1,
	PhysicsObject& box2, float w2, float h2, CollisionData& cd)
{
	Vector3 min1 = box1.pos - Vector3(w1, h1, 0.f);
	Vector3 max1 = box1.pos + Vector3(w1, h1, 0.f);
	Vector3 min2 = box2.pos - Vector3(w2, h2, 0.f);
	Vector3 max2 = box2.pos + Vector3(w2, h2, 0.f);

	if (!OverlapAABB2AABB(min1, max1, min2, max2))
		return false;

	// compute overlap on each axis
	float overlapX = std::min(max1.x, max2.x) - std::max(min1.x, min2.x);
	float overlapY = std::min(max1.y, max2.y) - std::max(min1.y, min2.y);

	// choose smallest penetration axis
	if (overlapX < overlapY)
	{
		// normal points from box1 -> box2 along x
		cd.collisionNormal = (box1.pos.x < box2.pos.x) ? Vector3(-1.f, 0.f, 0.f) : Vector3(1.f, 0.f, 0.f);
		cd.penetration = overlapX;
	}
	else
	{
		cd.collisionNormal = (box1.pos.y < box2.pos.y) ? Vector3(0.f, -1.f, 0.f) : Vector3(0.f, 1.f, 0.f);
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
bool OverlapCircle2Line(const Vector3& circlePos, float radius,
	const Vector3& lineStart,
	const Vector3& lineEnd)
{
	Vector3 lineTangent = lineEnd - lineStart;
	if (lineTangent.IsZero()) return false;  // not a valid segment

	float lineLength = lineTangent.Length();
	lineTangent.Normalize();

	// normal to the line
	Vector3 lineNormal{ -lineTangent.y, lineTangent.x, 0.f };

	// distance from circle center to infinite line
	float distToLine = (circlePos - lineStart).Dot(lineNormal);

	if (fabs(distToLine) > radius)
		return false;

	// projection along line tangent
	Vector3 lineVec = circlePos - lineStart;
	float projectedDist = lineVec.Dot(lineTangent);

	// check against segment endpoints if outside
	if (projectedDist > lineLength)
	{
		Vector3 diff = circlePos - lineEnd;
		float distSq = diff.LengthSquared();
		if (distSq > radius * radius)
			return false;
	}
	else if (projectedDist < 0)
	{
		Vector3 diff = circlePos - lineStart;
		float distSq = diff.LengthSquared();
		if (distSq > radius * radius)
			return false;
	}

	return true;
}

// Circle vs OBB (implement as circle vs AABB aligned with axes centered on box.pos using half-extents w,h)
bool OverlapCircle2OBB(PhysicsObject& circle, float radius, PhysicsObject& box, float w, float h, CollisionData& cd)
{
	Vector3 min = box.pos - Vector3(w, h, 0.f);
	Vector3 max = box.pos + Vector3(w, h, 0.f);

	// closest point on AABB to circle center
	float cx = std::max(min.x, std::min(circle.pos.x, max.x));
	float cy = std::max(min.y, std::min(circle.pos.y, max.y));
	Vector3 closest(cx, cy, 0.f);

	Vector3 diff = circle.pos - closest;
	float dist2 = diff.LengthSquared();

	if (dist2 > radius * radius)
		return false;

	cd.pObj1 = &circle;
	cd.pObj2 = &box;
	cd.contactPoint = closest;

	if (dist2 <= 1e-8f)
	{
		cd.collisionNormal = Vector3(0.f, 1.f, 0.f);
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
void ResolveCircle2StaticLine(PhysicsObject& ball, float radius, const Vector3& lineStart, const Vector3& lineEnd)
{
	Vector3 lineTangent = lineEnd - lineStart;
	float lineLength = lineTangent.Length();
	if (lineLength <= 1e-6f) return;
	lineTangent.Normalize();

	Vector3 lineVec = ball.pos - lineStart;
	float projectedDist = lineVec.Dot(lineTangent);

	Vector3 normal;
	float penetrationDist = 0.f;

	// ball beyond line end
	if (projectedDist > lineLength)
	{
		Vector3 closest = lineEnd;
		Vector3 fromClosest = ball.pos - closest;
		float dist = fromClosest.Length();
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = Vector3(-lineTangent.y, lineTangent.x, 0.f);
			normal.Normalize();
			penetrationDist = radius;
		}
	}
	// ball before line start
	else if (projectedDist < 0.f)
	{
		Vector3 closest = lineStart;
		Vector3 fromClosest = ball.pos - closest;
		float dist = fromClosest.Length();
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = Vector3(-lineTangent.y, lineTangent.x, 0.f);
			normal.Normalize();
			penetrationDist = radius;
		}
	}
	// ball projects onto segment
	else
	{
		Vector3 closest = lineStart + lineTangent * projectedDist;
		Vector3 fromClosest = ball.pos - closest;
		float dist = fromClosest.Length();
		if (dist > 1e-6f)
		{
			normal = fromClosest / dist;
			penetrationDist = radius - dist;
		}
		else
		{
			normal = Vector3(-lineTangent.y, lineTangent.x, 0.f);
			normal.Normalize();
			penetrationDist = radius;
		}
	}

	// resolve penetration
	if (penetrationDist > 0.f)
	{
		ball.pos += normal * penetrationDist;
		// remove normal component of velocity
		float vn = ball.vel.Dot(normal);
		ball.vel -= normal * vn;
	}
}