#pragma once
#include "PhysicsObject.h"

class DuckTarget : public PhysicsObject
{
private:
    // Movement path
    glm::vec3 startPos;
    glm::vec3 endPos;

    // Movement control
    float moveSpeed;          // units per second
    bool movingToEnd = true;

    // Lifetime
    int repeatsRemaining;     // how many back-and-forths left
    bool active = true;

    int maxRepeats = 3;
    int currentRepeats = 0;

    // Hit behavior
    bool wasHit = false;
    float hitImpulseStrength;

    int scoreValue;
public:
    DuckTarget(glm::vec3 start,glm::vec3 end,glm::vec3 size,float speed,int repeats, int value);
    ~DuckTarget();

    void Update(float dt);
    bool OnHit();
    bool IsActive();

    int GetScoreValue();
};

