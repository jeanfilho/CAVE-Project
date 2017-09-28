#include "Scene.h"

Scene::Scene()
{	
	riverSections = std::vector<GameObject>();
	monkeys = std::vector<Monkey>();

	coconutSpeed = 1000;
	isCoconutActive = false;
	coconutRadius = 7.5f;

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

	
	/* Load boat */
	boat = GameObject(SceneFileHandler::the()->read("models/boat.obj"), "models/Wood.png");
	world->addChild(boat.getNode());

	/* Load river sections */
	river = GameObject(SceneFileHandler::the()->read("models/river.obj"), "models/Water.png");
	river.addUniformVariable("ElapsedTime", 0.0f);
	river.setVertexProgram(_water_vertex_shader);

	/* Load jungle mesh */
	scenery = GameObject(SceneFileHandler::the()->read("models/scenery.obj"), "models/Scenery.png");
	for(int i = 0; i < 7; i++)
	{
		ComponentTransformRecPtr trans = ComponentTransform::create();
		trans->setTranslation(Vec3f(0, -10, -i * riverLength));
		GameObject riverSection = GameObject(trans, OSG::cloneTree(river.getGeometryNode()), "models/Water.png");
		riverSection.setMaterialNode(river.getMaterialNode());
		riverSection.addChild(OSG::cloneTree(scenery.getMaterialNode()));
		riverSections.push_back(riverSection);
		world->addChild(riverSection.getNode());
	}
	
	/* Load Coconut */
	coconut = GameObject(SceneFileHandler::the()->read("models/coconut.obj"), "models/coconut.png");
	coconut.setTranslation(Vec3f(0,10,200));
	world->addChild(coconut.getNode());

	/* Load monkey */
	float monkeyCapsuleHeight = 50;
	float monkeyCapsuleRadius = 10;
	monkeys.push_back(Monkey(makeBox(monkeyCapsuleRadius * 2,monkeyCapsuleHeight, monkeyCapsuleRadius * 2, 1, 1, 1), monkeyCapsuleRadius, monkeyCapsuleHeight, "models/coconut.png"));
	for(int i = 0; i < monkeys.size(); i++)
	{
		monkeys[i].setTranslation(Vec3f(-50, 100, -50));
		monkeys[i].setMaterialNode(monkeys[0].getMaterialNode());
		world->addChild(monkeys[i].getNode());
	}
		
	/* Build tree */
	base = makeNodeFor(dirLight1);
	base->addChild(world);
}

void Scene::update(float deltaTime, Matrix4f viewMatrix)
{
	boat.updateUniformVariable("ViewMatrix", viewMatrix);
	scenery.updateUniformVariable("ViewMatrix", viewMatrix);
	river.updateUniformVariable("ViewMatrix", viewMatrix);
	river.updateUniformVariable("ElapsedTime", TimeManager::elapsedTime());
	coconut.updateUniformVariable("ViewMatrix", viewMatrix);
	animateScenery(deltaTime);
	animateMonkeys(deltaTime);
	animateCoconut(deltaTime);
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

void Scene::animateCoconut(float deltaTime)
{
	if(isCoconutActive)
	{
		coconutVelocity += Vec3f(0, -gravity * deltaTime, 0);
		coconut.setTranslation(coconut.getTranslation() + coconutVelocity * deltaTime);
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
	if(!isCoconutActive)
		return;

	/* Test coconut against monkeys - Capsule-Sphere intersection */
	for(int i = 0; i < monkeys.size(); i++)
	{
		if(monkeyCoconutIntersection(&monkeys[i]))
		{
			std::cout << "Monkey hit!" << std::endl;
			Vec3f temp = monkeys[i].getTranslation() - coconut.getTranslation(); 
			monkeys[i].isHit = true;
			monkeys[i].Velocity = Vec3f(temp.x(), 5, temp.z());
			monkeys[i].Velocity.normalize();
			monkeys[i].Velocity *= coconutSpeed / 2;

			coconut.setTranslation(Vec3f(0,10,200));
			isCoconutActive = false;
		}
	}

	/* Test coconut against environment - check if sphere Y is under 0 */
	if(coconut.getTranslation().y() < -coconutRadius)
	{
		coconut.setTranslation(Vec3f(0,10,200));
		isCoconutActive = false;
	}
}

void Scene::throwCoconut(Vec3f position, Vec3f direction)
{
	if(isCoconutActive)
		return;

	isCoconutActive = true;
	coconut.setTranslation(position);
	coconutVelocity = direction * coconutSpeed;
}

bool Scene::monkeyCoconutIntersection(Monkey* monkey)
{
	Vec3f capsuleCenter = monkey->getTranslation();
	float top = capsuleCenter.y() + monkey->getCapsuleHeight()/2 - monkey->getCapsuleRadius();
	float bottom = capsuleCenter.y() - monkey->getCapsuleHeight()/2 + monkey->getCapsuleRadius();
	float distance;
	if(top - coconut.getTranslation().y() >= 0 && bottom - coconut.getTranslation().y() <= 0)
	{
		Vec3f temp = capsuleCenter - coconut.getTranslation();
		distance = Vec2f(temp.x(), temp.z()).length();
	}
	else
	{
		if(top - coconut.getTranslation().y() < 0)
		{
			distance = (capsuleCenter + Vec3f(0, monkey->getCapsuleHeight()/2 - monkey->getCapsuleRadius(), 0) - coconut.getTranslation()).length();
		}
		else
		{
			distance = (capsuleCenter + Vec3f(0, - monkey->getCapsuleHeight()/2 + monkey->getCapsuleRadius(), 0) - coconut.getTranslation()).length();
		}
	}		

	if(distance < monkey->getCapsuleRadius() + coconutRadius)
		return true;
	else
		return false;
}