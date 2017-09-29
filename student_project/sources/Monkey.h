#pragma once
#include <OpenSG/OSGComponentTransform.h>
#include "GameObject.h"

class Monkey : public GameObject
{
private:
	float capsuleRadius;
	float capsuleHeight;

public:
	Vec3f Velocity;
	bool isHit;

	Monkey(NodeRecPtr geometry, float capsuleRadius, float capsuleHeight) : GameObject(geometry)
	{
		this->capsuleHeight = capsuleHeight;
		this->capsuleRadius = capsuleRadius;
		isHit = false;
		Velocity = Vec3f(0,0,0);
	}
	~Monkey(){};

	float getCapsuleHeight()
	{
		return capsuleHeight;
	}

	float getCapsuleRadius()
	{
		return capsuleRadius;
	}	
};