#pragma once

#include <vector>

#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGMultiDisplayWindow.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGDirectionalLight.h>
#include <OpenSG/OSGComponentTransform.h>

#include "GameObject.h"

OSG_USING_NAMESPACE

class Scene
{
private:
	NodeRecPtr base;
	std::vector<GameObject> riverSections;

	float riverLength;
	float boatSpeed;

public:
	Scene();
	~Scene();

	void initialize();
	void update(float deltaTime);
	void moveNavSphere(Vec3f pos);
	NodeTransitPtr getBase();

};

