#pragma once

#include <OpenSG\OSGComponentTransform.h>
#include "GameObject.h"

class Monkey : public GameObject
{
private:
	float capsuleRadius;
	float capsuleHeight;
	float capsuleYOffset;

public:
	enum State
	{
		Hit,
		OnRock,
		OnBoat,
		Jumping,
		Inactive
	};

	Vec3f Velocity;
	float BaseY;
	float jumpDeltaX;
	Vec3f jumpTarget;
	Vec3f jumpDirection;
	State monkeyState;

	Monkey():GameObject(){} 

	Monkey(NodeRecPtr geometry, float capsuleRadius, float capsuleHeight, float capsuleYOffset, std::string textureImage) : GameObject(geometry, textureImage)
	{
		this->capsuleHeight = capsuleHeight;
		this->capsuleRadius = capsuleRadius;
		this->capsuleYOffset = capsuleYOffset;
		monkeyState = State::Inactive;
		Velocity = Vec3f(0,0,0);
	}
	Monkey(NodeRecPtr material, float capsuleRadius, float capsuleHeight, float capsuleYOffset) : GameObject(material)
	{
		this->capsuleHeight = capsuleHeight;
		this->capsuleRadius = capsuleRadius;
		this->capsuleYOffset = capsuleYOffset;
		monkeyState = State::Inactive;
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

	float getCapsuleYOffset()
	{
		return capsuleYOffset;
	}
};