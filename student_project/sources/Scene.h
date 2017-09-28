#pragma once

#include <vector>

#include <OpenSG\OSGGLUT.h>
#include <OpenSG\OSGConfig.h>
#include <OpenSG\OSGSimpleGeometry.h>
#include <OpenSG\OSGGLUTWindow.h>
#include <OpenSG\OSGMultiDisplayWindow.h>
#include <OpenSG\OSGSceneFileHandler.h>
#include <OpenSG\OSGDirectionalLight.h>
#include <OpenSG\OSGComponentTransform.h>
#include <OpenSG\OSGSimpleMaterial.h>
#include <OpenSG\OSGMaterialGroup.h>
#include <OpenSG\OSGSkyBackground.h>
#include <OpenSG\OSGTextureObjChunk.h>
#include <OpenSG\OSGMaterialGroup.h>
#include <OpenSG\OSGChunkMaterial.h>
#include <OpenSG\OSGSimpleSHLChunk.h>
#include <OpenSG\OSGImage.h>

#include "..\shader\Shaders.h"
#include "GameObject.h"
#include "Monkey.h"
#include "TimeManager.h"

OSG_USING_NAMESPACE

class Scene
{
private:
	NodeRecPtr base;	
	GameObject boat, scenery, river;
		
	std::vector<GameObject> riverSections;
	float riverLength;
	float boatSpeed;
	float gravity;

	std::vector<Monkey> monkeys;
	
	GameObject coconut;
	bool isCoconutActive;
	float coconutSpeed;
	float coconutRadius;
	Vec3f coconutVelocity;
		
	void animateScenery(float deltaTime);
	void animateMonkeys(float deltaTime);
	void animateCoconut(float deltaTime);
	void checkCollisions(float deltaTime);
	bool monkeyCoconutIntersection(Monkey* monkey);

public:
	Scene();
	~Scene();

	void initialize();
	void update(float deltaTime, Matrix4f viewMatrix);
	void moveNavSphere(Vec3f pos);
	void throwCoconut(Vec3f position, Vec3f direction);
	NodeTransitPtr getBase();

};

