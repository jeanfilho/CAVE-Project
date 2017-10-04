#include "Scene.h"

Scene::Scene()
{	
	riverSections = std::vector<GameObject>();
	monkeys = std::vector<Monkey>();
	crocodiles = std::vector<Monkey>();

	coconutSpeed = 1000;
	isCoconutActive = false;
	coconutRadius = 7.5f;

	gravity = 9.8f * 100; /* cm/S² */

	riverLength = 1000;
	boatSpeed = 100;
	numberOfMonkeys = 2;
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
	river.setVertexProgram(_water_vertex_shader);

	/* Load jungle mesh */
	scenery = GameObject(SceneFileHandler::the()->read("models/Scenery.obj"), "models/Scenery.png");
	for(int i = 0; i < 7; i++)
	{
		ComponentTransformRecPtr trans = ComponentTransform::create();
		trans->setTranslation(Vec3f(0, -10, -i * riverLength));
		GameObject riverSection = GameObject(trans, OSG::cloneTree(river.getMaterialNode()));
		GameObject scenerySection = GameObject(OSG::cloneTree(scenery.getMaterialNode()));
		riverSection.addChild(scenerySection.getNode());
		riverSections.push_back(riverSection);
		world->addChild(riverSection.getNode());
	}

	/* Load Coconut */
	coconut = GameObject(SceneFileHandler::the()->read("models/coconut.obj"), "models/Coconut.png");
	coconut.setTranslation(Vec3f(0,10,200));
	world->addChild(coconut.getNode());

	/* Load monkey */
	float monkeyCapsuleHeight = 70;
	float monkeyCapsuleRadius = 15;
	monkey = GameObject(SceneFileHandler::the()->read("models/monkey.obj"), "models/Monkey.png");
	for(int i = 0; i < numberOfMonkeys; i++)
	{
		monkeys.push_back(Monkey(OSG::cloneTree(monkey.getMaterialNode()), monkeyCapsuleRadius, monkeyCapsuleHeight, monkeyCapsuleHeight/2));
		monkeys[i].setTranslation(Vec3f(0, 0, 1000));
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
	coconut.updateUniformVariable("ViewMatrix", viewMatrix);
	monkey.updateUniformVariable("ViewMatrix", viewMatrix);
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
		switch(monkeys[i].monkeyState)
		{
		case Monkey::State::Hit:

			monkeys[i].Velocity += Vec3f(0,-gravity*deltaTime,0);
			monkeys[i].setTranslation(monkeys[i].getTranslation() + monkeys[i].Velocity * deltaTime);
			if(monkeys[i].getTranslation().y() < -150)
			{
				monkeys[i].setTranslation(Vec3f(0, 0, 1000));
				monkeys[i].setRotation(Quaternion::identity());
				monkeys[i].monkeyState = Monkey::State::Inactive;
			}
			break;
		case Monkey::State::OnBoat:
		case Monkey::State::OnRock:
			monkeys[i].setTranslation(Vec3f(monkeys[i].getTranslation().x(), 10 * osgPow(sin(TimeManager::elapsedTime()*20), 2), monkeys[i].getTranslation().z()));
			break;
		case Monkey::State::Jumping:
			break;
		case Monkey::State::Inactive:
			break;
		}
	}
}

void Scene::animateCoconut(float deltaTime)
{
	if(isCoconutActive)
	{
		coconutVelocity += Vec3f(0, -gravity * deltaTime, 0);
		coconut.setTranslation(coconut.getTranslation() + coconutVelocity * deltaTime);
		coconut.setRotation(coconut.getRotation() * Quaternion(Vec3f(1,0,1), osgDegree2Rad(500 * deltaTime)));
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

	/* Test coconut against active monkeys - Capsule-Sphere intersection */
	for(int i = 0; i < monkeys.size(); i++)
	{
		if(monkeys[i].monkeyState == Monkey::State::Inactive)
			continue;

		if(monkeyCoconutIntersection(&monkeys[i]))
		{
			std::cout << "Monkey hit!" << std::endl;
			Vec3f temp = monkeys[i].getTranslation() - coconut.getTranslation(); 
			monkeys[i].monkeyState = Monkey::State::Hit;
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
		coconut.setTranslation(Vec3f(0,10,500));
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
	Vec3f capsuleCenter = monkey->getTranslation() + Vec3f(0,monkey->getCapsuleYOffset(),0);
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