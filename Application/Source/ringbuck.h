#pragma once
#include "PhysicsObject.h"

class ringbuck
{
public:
	//size
	float radius = 2.f;
	float sradius = 1.9f;
	float wradius = radius * 2;
	float height = 3.f;
	float wall_thin = radius - sradius;
	//object
	PhysicsObject bucket;
	PhysicsObject wall[4];
	//functions
	ringbuck();
	~ringbuck();

	
};

