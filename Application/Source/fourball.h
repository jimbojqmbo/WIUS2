#pragma once
#include "PhysicsObject.h"

class fourball
{
public:
	//visibility
	bool visible = false;
	//size
	float radius = 1;
	//object
	PhysicsObject ball;
	//functions
	fourball();
	~fourball();
};

