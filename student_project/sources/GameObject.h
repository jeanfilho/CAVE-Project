#pragma once

#include <OpenSG/OSGComponentTransform.h>

OSG_USING_NAMESPACE

class GameObject
{
private:
	NodeRecPtr geometry;
	NodeRecPtr transform;

public:
	GameObject(ComponentTransformRecPtr transform, NodeRecPtr geometry)
	{
		this->transform = makeNodeFor(transform);
		this->transform->addChild(geometry);
	}
	~GameObject(){}

	void setTranslation(Vec3f pos)
	{
		dynamic_cast<ComponentTransform*>(transform->getCore())->setTranslation(pos);
	}
	void setRotation(Quaternion quat)
	{
		dynamic_cast<ComponentTransform*>(transform->getCore())->setRotation(quat);
	}

	Vec3f getTranslation()
	{
		return dynamic_cast<ComponentTransform*>(transform->getCore())->getTranslation();
	}

	Quaternion getRotation()
	{
		return dynamic_cast<ComponentTransform*>(transform->getCore())->getRotation();
	}

	NodeRecPtr getNode()
	{
		return transform;
	}
};