#pragma once
#include "PhysicsObject.h"

class fourball
{
public:
	//rizzy varibles
	bool thrown = false;
	bool in = false;
	//size
	float radius = 0.5;
	//object
	PhysicsObject ball;
	//functions
	fourball();
	~fourball();
};

