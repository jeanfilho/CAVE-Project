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
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGMaterialGroup.h>
#include <OpenSG\OSGSkyBackground.h>
#include <OpenSG\OSGImage.h>
#include <OpenSG\OSGTextureObjChunk.h>
#include <OpenSG\OSGMaterialGroup.h>
#include <OpenSG\OSGChunkMaterial.h>
#include <OpenSG\OSGSimpleSHLChunk.h>
#include <OpenSG\OSGImage.h>


#include "..\shader\Shader.h"
#include "GameObject.h"
#include "Monkey.h"

OSG_USING_NAMESPACE

class Scene
{
private:
	NodeRecPtr base;	
	GameObject boat;
		
	std::vector<GameObject> riverSections;
	float riverLength;
	float boatSpeed;
	float gravity;

	std::vector<Monkey> monkeys;
	
	GameObject waterBalloon;
	bool isBalloonActive;
	float balloonSpeed;
	float balloonRadius;
	Vec3f balloonVelocity;
		
	void animateScenery(float deltaTime);
	void animateMonkeys(float deltaTime);
	void animateBalloon(float deltaTime);
	void checkCollisions(float deltaTime);
	bool monkeyBalloonIntersection(Monkey* monkey);

public:
	Scene();
	~Scene();

	void initialize();
	void update(float deltaTime);
	void moveNavSphere(Vec3f pos);
	void throwBalloon(Vec3f position, Vec3f direction);
	NodeTransitPtr getBase();

};

