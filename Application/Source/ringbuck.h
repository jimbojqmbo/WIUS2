#pragma once
#include "PhysicsObject.h"

class ringbuck
{
public:
	//size
	float radius = 2.f;
	float sradius = 1.9f;
	float height = 1.5f;
	//object
	PhysicsObject bucket;
	PhysicsObject wall[4];
	//functions
	ringbuck();
	~ringbuck();
};

