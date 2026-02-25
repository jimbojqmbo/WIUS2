#pragma once
#include "PhysicsObject.h"

class ringbuck
{
public:
	//size
	float radius = 1.5f;
	float height = 1.5f;
	//object
	PhysicsObject bucket;
	//functions
	ringbuck();
	~ringbuck();
};

