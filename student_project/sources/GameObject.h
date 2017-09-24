#pragma once

#include <OpenSG/OSGComponentTransform.h>
#include <OpenSG\OSGChunkMaterial.h>

OSG_USING_NAMESPACE

class GameObject
{
private:
	NodeRecPtr geometry;
	NodeRecPtr transform;

public:
	GameObject(){}

	GameObject(ComponentTransformRecPtr transform, NodeRecPtr geometry)
	{
		this->transform = makeNodeFor(transform);
		this->transform->addChild(geometry);
	}
	GameObject(NodeRecPtr geometry)
	{
		ComponentTransformRecPtr transform = ComponentTransform::create();
		transform->setTranslation(Vec3f(0,0,0));
		this->transform = makeNodeFor(transform);
		this->transform->addChild(geometry);
	}
	GameObject(Vec3f position, NodeRecPtr geometry)
	{
		ComponentTransformRecPtr transform =  ComponentTransform::create();
		transform->setTranslation(position);
		this->transform = makeNodeFor(transform);
		this->transform->addChild(geometry);
	}
	GameObject(Vec3f position, Quaternion rotation, NodeRecPtr geometry)
	{
		ComponentTransformRecPtr transform =  ComponentTransform::create();
		transform->setRotation(rotation);
		transform->setTranslation(position);
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