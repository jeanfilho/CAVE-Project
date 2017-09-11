#pragma once

#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGMultiDisplayWindow.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGDirectionalLight.h>

OSG_USING_NAMESPACE

class Scene
{
private:
	NodeRecPtr base;

public:
	Scene();
	~Scene();

	void initialize();
	NodeTransitPtr getBase();

};

