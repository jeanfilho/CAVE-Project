#pragma once

#include <OpenSG\OSGComponentTransform.h>
#include "GameObject.h"

class Crocodile : public GameObject
{
private:
	Vec3f boxDimensions;

public:
	bool isActive;

	Crocodile():GameObject(){}
	~Crocodile(){}

	Crocodile(NodeRecPtr geometry, Vec3f boxDimensions, std::string textureImage) : GameObject(geometry, textureImage)
	{
		this->boxDimensions = boxDimensions;
	}

	Crocodile(NodeRecPtr geometry, Vec3f boxDimensions) : GameObject(geometry)
	{
		this->boxDimensions = boxDimensions;
	}

	Vec3f getBoxDimensions()
	{
		return boxDimensions;
	}
};