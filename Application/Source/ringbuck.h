#pragma once
#include "PhysicsObject.h"

class ringbuck
{
public:
	//size
	float radius = 2.f;
	float sradius = 1.f;
	float height = 1.5f;
	//object
	PhysicsObject bucket;
	//functions
	ringbuck();
	~ringbuck();
};

