#include "DuckTarget.h"

DuckTarget::DuckTarget(glm::vec3 start, glm::vec3 end, glm::vec3 size, float speed, int repeats, int value)
{
    hitImpulseStrength = 2.f;
    pos = start;
	startPos = start;
	endPos = end;
	moveSpeed = speed;
	repeatsRemaining = repeats;
    sizeX = size.x;
    sizeY = size.y;
    sizeZ = size.z;
    scoreValue = value;
}

DuckTarget::~DuckTarget()
{
}

void DuckTarget::Update(float dt)
{
    if (!active) return;

    glm::vec3 targetPos = movingToEnd ? endPos : startPos;
    glm::vec3 dir = targetPos - pos;
    float dist = glm::length(dir);

    if (dist < 0.05f)
    {
        movingToEnd = !movingToEnd;

        if (!movingToEnd)
        {
            repeatsRemaining--;
            if (repeatsRemaining <= 0)
            {
                active = false;
                return;
            }
        }
    }
    else
    {
        dir = glm::normalize(dir);
        pos += dir * moveSpeed * dt;
    }

	UpdatePhysics(dt);
}

bool DuckTarget::OnHit()
{
    if (wasHit) return false;

    wasHit = true;

    accel += glm::vec3(0, -50.f, 0);

    return true;
}

bool DuckTarget::IsActive()
{
    return active;
}

int DuckTarget::GetScoreValue()
{
    return scoreValue;
}
