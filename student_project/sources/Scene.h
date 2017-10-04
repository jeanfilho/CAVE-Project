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
#include <OpenSG\OSGImageForeground.h>

#include "..\shader\Shaders.h"
#include "GameObject.h"
#include "Monkey.h"
#include "Crocodile.h"
#include "TimeManager.h"

OSG_USING_NAMESPACE

class Scene
{
private:
	ImageForegroundRecPtr fg;
	ImageRecPtr winImg, loseImg, startImg;
	NodeRecPtr base;	
	GameObject boat, scenery, river, monkey, croc;
		
	std::vector<GameObject> riverSections;
	float boatWidth, boatLength;
	float riverLength;
	float boatSpeed;
	float gravity;
	float startBlock;
	float levelStartTime;
	int score;
	int windowX, windowY;
	int surviveTime;


	float healthPoints;
	
	std::vector<Monkey> monkeys;
	int numberOfMonkeys;
	Vec3f rightRock, leftRock;

	
	std::vector<Crocodile> crocodiles;
	int numberOfCrocodiles;
	
	GameObject coconut;
	bool isCoconutActive;
	float coconutSpeed;
	float coconutRadius;
	Vec3f coconutVelocity;
		
	void animateScenery(float deltaTime);
	void animateMonkeys(float deltaTime);
	void animateCoconut(float deltaTime);
	void animateCrocodile(float deltaTime);
	void checkCollisions(float deltaTime);
	void displayForeground();
	bool monkeyCoconutIntersection(Monkey* monkey);
	bool crocodileCoconutIntersection(Crocodile* croc);

public:
	bool isPlay;

	Scene();
	~Scene();

	void initialize();
	void update(float deltaTime, Matrix4f viewMatrix);
	void moveNavSphere(Vec3f pos);
	void throwCoconut(Vec3f position, Vec3f direction);
	void start();
	float getScore();
	NodeTransitPtr getBase();
	ImageForegroundRecPtr getImageForeground();

};

