#include "Scene.h"

Scene::Scene()
{	
	riverSections = std::vector<GameObject>();
	monkeys = std::vector<Monkey>();

	balloonSpeed = 1000;
	isBalloonActive = false;
	balloonRadius = 10;

	gravity = 9.8f * 100; /* cm/s² */

	riverLength = 1000;
	boatSpeed = 100;
}


Scene::~Scene()
{
}
void Scene::initialize()
{
	/*Lit Scene*/
	NodeRecPtr world = Node::create();
	world->setCore(Group::create());

	/* Main Directional Light */
	DirectionalLightRecPtr dirLight1 = DirectionalLight::create();
	dirLight1->setDirection(-.5f,1,.5f);
	dirLight1->setDiffuse(Color4f(1,1,1,1));
	dirLight1->setAmbient(Color4f(.5,.5,.5,1));
	dirLight1->setSpecular(Color4f(.1f,.1f,.1f,0.2f));	
	dirLight1->setBeacon(world);

	/* Create chunk material base for custom shaders */
	ChunkMaterialRecPtr cmat = ChunkMaterial::create();
	TextureObjChunkRecPtr tex = TextureObjChunk::create();
	ImageRecPtr img = Image::create();
	tex->setImage(img);
	cmat->addChunk(tex);
	MaterialGroupRecPtr materialGroup = MaterialGroup::create();
	materialGroup->setMaterial(cmat);
	NodeRecPtr materialBase = makeNodeFor(materialGroup);
	
	/* Load boat */
	img->read("models/Wood.png");
	NodeRecPtr boatMat = OSG::deepCloneTree(materialBase);
	boatSHL = SimpleSHLChunk::create();
	boatSHL->setFragmentProgram(_fragment_shader);
	boatSHL->setVertexProgram(_vertex_shader);
	boatSHL->addUniformVariable("ViewMatrix", Matrix4f());
	dynamic_cast<ChunkMaterial*>(dynamic_cast<MaterialGroup*>(boatMat->getCore())->getMaterial())->addChunk(boatSHL);
	boatMat->addChild(SceneFileHandler::the()->read("models/boat.obj"));
	boat = GameObject(boatMat);
	world->addChild(boat.getNode());

	/* Load river sections */
	img->read("models/Water.png");
	NodeRecPtr riverMat = OSG::deepCloneTree(materialBase);
	riverSHL = SimpleSHLChunk::create();
	riverSHL->setFragmentProgram(_fragment_shader);
	riverSHL->setVertexProgram(_water_vertex_shader);
	riverSHL->addUniformVariable("ViewMatrix", Matrix4f());
	riverSHL->addUniformVariable("ElapsedTime", 0.0f);
	dynamic_cast<ChunkMaterial*>(dynamic_cast<MaterialGroup*>(riverMat->getCore())->getMaterial())->addChunk(riverSHL);
	riverMat->addChild(SceneFileHandler::the()->read("models/river.obj"));

	/* Load jungle mesh */
	img->read("models/Scenery.png");
	img->setMipMapCount(0);
	img->removeMipmap();
	tex->setMinFilter(GL_LINEAR);
	tex->setMagFilter(GL_LINEAR);
	NodeRecPtr sceneryMat = OSG::deepCloneTree(materialBase);
	scenerySHL = SimpleSHLChunk::create();
	scenerySHL->setFragmentProgram(_fragment_shader);
	scenerySHL->setVertexProgram(_vertex_shader);
	scenerySHL->addUniformVariable("ViewMatrix", Matrix4f());
	dynamic_cast<ChunkMaterial*>(dynamic_cast<MaterialGroup*>(sceneryMat->getCore())->getMaterial())->addChunk(scenerySHL);
	sceneryMat->addChild(SceneFileHandler::the()->read("models/scenery.obj"));
	for(int i = 0; i < 7; i++)
	{
		ComponentTransformRecPtr trans = ComponentTransform::create();
		trans->setTranslation(Vec3f(0, -10, -i * riverLength));
		GameObject riverSection = GameObject(trans, OSG::cloneTree(riverMat));
		riverSection.addChild(OSG::cloneTree(sceneryMat));
		riverSection.addChild(OSG::cloneTree(sceneryMat));
		riverSections.push_back(riverSection);
		world->addChild(riverSection.getNode());
	}
	
	/* Load balloon */
	//TODO proper model
	waterBalloon = GameObject(makeSphere(2, balloonRadius));
	world->addChild(waterBalloon.getNode());

	/* Load monkey */
	float monkeyCapsuleHeight = 50;
	float monkeyCapsuleRadius = 10;
	monkeys.push_back(Monkey(makeBox(monkeyCapsuleRadius * 2,monkeyCapsuleHeight, monkeyCapsuleRadius * 2, 1, 1, 1), monkeyCapsuleRadius, monkeyCapsuleHeight));
	for(int i = 0; i < monkeys.size(); i++)
	{
		monkeys[i].setTranslation(Vec3f(-50, 100, -50));
		world->addChild(monkeys[i].getNode());
	}
		
	/* Build tree */
	base = makeNodeFor(dirLight1);
	base->addChild(world);
}

void Scene::update(float deltaTime, Matrix4f viewMatrix)
{
	boatSHL->updateUniformVariable("ViewMatrix", viewMatrix);
	scenerySHL->updateUniformVariable("ViewMatrix", viewMatrix);
	riverSHL->updateUniformVariable("ViewMatrix", viewMatrix);
	riverSHL->updateUniformVariable("ElapsedTime", TimeManager::elapsedTime());
	animateScenery(deltaTime);
	animateMonkeys(deltaTime);
	animateBalloon(deltaTime);
	checkCollisions(deltaTime);
}

NodeTransitPtr Scene::getBase()
{
	return NodeTransitPtr(base);
}

void Scene::animateMonkeys(float deltaTime)
{
	for(int i = 0; i < monkeys.size(); i++)
	{
		if(monkeys[i].isHit)
		{
			monkeys[i].Velocity += Vec3f(0,-gravity*deltaTime,0);
			monkeys[i].setTranslation(monkeys[i].getTranslation() + monkeys[i].Velocity * deltaTime);
			if(monkeys[i].getTranslation().y() < -100)
			{
				monkeys[i].setTranslation(Vec3f(-50, 100, -50));
				monkeys[i].isHit = false;
			}
		}
	}
}

void Scene::animateBalloon(float deltaTime)
{
	if(isBalloonActive)
	{
		balloonVelocity += Vec3f(0, -gravity * deltaTime, 0);
		waterBalloon.setTranslation(waterBalloon.getTranslation() + balloonVelocity * deltaTime);
	}	
}

void Scene::animateScenery(float deltaTime)
{
	for(int i = 0; i < riverSections.size(); i++)
	{
		riverSections[i].setTranslation(riverSections[i].getTranslation() + boatSpeed * deltaTime * Vec3f(0,0,1));
		if(riverSections[i].getTranslation().z() > riverLength)
			riverSections[i].setTranslation(riverSections[i].getTranslation() + Vec3f(0,0,-((int)riverSections.size()) * riverLength));
	}
}

void Scene::checkCollisions(float deltaTime)
{
	if(!isBalloonActive)
		return;

	/* Test balloon against monkeys - Capsule-Sphere intersection */
	for(int i = 0; i < monkeys.size(); i++)
	{
		if(monkeyBalloonIntersection(&monkeys[i]))
		{
			std::cout << "Monkey hit!" << std::endl;
			Vec3f temp = monkeys[i].getTranslation() - waterBalloon.getTranslation(); 
			monkeys[i].isHit = true;
			monkeys[i].Velocity = Vec3f(temp.x(), 5, temp.z());
			monkeys[i].Velocity.normalize();
			monkeys[i].Velocity *= balloonSpeed / 2;

			waterBalloon.setTranslation(Vec3f(0,10,200));
			isBalloonActive = false;
		}
	}

	/* Test balloon against environment - check if sphere is outside AABB */
	if(waterBalloon.getTranslation().y() < -balloonRadius)
	{
		waterBalloon.setTranslation(Vec3f(0,10,200));
		isBalloonActive = false;
	}
}

void Scene::throwBalloon(Vec3f position, Vec3f direction)
{
	if(isBalloonActive)
		return;

	isBalloonActive = true;
	waterBalloon.setTranslation(position);
	balloonVelocity = direction * balloonSpeed;
}

bool Scene::monkeyBalloonIntersection(Monkey* monkey)
{
	Vec3f capsuleCenter = monkey->getTranslation();
	float top = capsuleCenter.y() + monkey->getCapsuleHeight()/2 - monkey->getCapsuleRadius();
	float bottom = capsuleCenter.y() - monkey->getCapsuleHeight()/2 + monkey->getCapsuleRadius();
	float distance;
	if(top - waterBalloon.getTranslation().y() >= 0 && bottom - waterBalloon.getTranslation().y() <= 0)
	{
		Vec3f temp = capsuleCenter - waterBalloon.getTranslation();
		distance = Vec2f(temp.x(), temp.z()).length();
	}
	else
	{
		if(top - waterBalloon.getTranslation().y() < 0)
		{
			distance = (capsuleCenter + Vec3f(0, monkey->getCapsuleHeight()/2 - monkey->getCapsuleRadius(), 0) - waterBalloon.getTranslation()).length();
		}
		else
		{
			distance = (capsuleCenter + Vec3f(0, - monkey->getCapsuleHeight()/2 + monkey->getCapsuleRadius(), 0) - waterBalloon.getTranslation()).length();
		}
	}		

	if(distance < monkey->getCapsuleRadius() + balloonRadius)
		return true;
	else
		return false;
}