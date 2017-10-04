#include "Scene.h"

Scene::Scene()
{	
	leftRock = Vec3f(-330, 95, -300);
	rightRock = Vec3f(300, 63, 60);

	coconutSpeed = 1000;
	isCoconutActive = false;
	coconutRadius = 7.5f;

	gravity = 9.8f * 100; /* cm/S² */

	boatWidth = 200;
	boatLength = 200;
	riverLength = 1000;
	boatSpeed = 300;
	numberOfMonkeys = 4;
	numberOfCrocodiles = 7;

	riverSections = std::vector<GameObject>();
	monkeys = std::vector<Monkey>();
	crocodiles = std::vector<Crocodile>();

	windowX = 800;
	windowY = 600;

	surviveTime = 90;
}


Scene::~Scene()
{
}

void Scene::initialize()
{
	startBlock = TimeManager::elapsedTime();

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
	river.addUniformVariable("SpeedFactor", 0.0f);

	/* Load jungle mesh */
	scenery = GameObject(SceneFileHandler::the()->read("models/Scenery.obj"), "models/Scenery.png");
	for(int i = 0; i < numberOfCrocodiles; i++)
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
		monkeys[i].monkeyState = Monkey::State::Inactive;
		world->addChild(monkeys[i].getNode());
	}

	/* Load Crocodile */
	Vec3f crocBox = Vec3f(45,23,100);
	croc = GameObject(SceneFileHandler::the()->read("models/crocodile.obj"), "models/Croc.png");
	for(int i = 0; i < numberOfCrocodiles; i++)
	{
		crocodiles.push_back(Crocodile(OSG::cloneTree(croc.getMaterialNode()), crocBox));
		crocodiles[i].setTranslation(Vec3f(0, 0, 1000));
		world->addChild(crocodiles[i].getNode());
	}

	/* Build tree */
	base = makeNodeFor(dirLight1);
	base->addChild(world);

	/* Load foreground */
	fg = ImageForeground::create();
	loseImg = Image::create();
	winImg = Image::create();
	startImg = Image::create();
	loseImg->read("textures/lose.png");
	winImg->read("textures/win.png");
	startImg->read("textures/start.png");
	displayForeground();
}

void Scene::start()
{	
	if(TimeManager::elapsedTime() - startBlock < 2)
		return;

	levelStartTime = TimeManager::elapsedTime();
	for(int i = 0; i < numberOfCrocodiles; i++)
	{		
		crocodiles[i].setTranslation(Vec3f(0, 0, 1000));
		crocodiles[i].crocState = Crocodile::State::Inactive;
	}
	for(int i = 0; i < numberOfMonkeys; i++)
	{
		monkeys[i].setTranslation(Vec3f(0, 0, 1000));
		monkeys[i].monkeyState = Monkey::State::Inactive;
	}
	for(int i = 0; i < numberOfCrocodiles; i++)
	{
		riverSections[i].setTranslation(Vec3f(0, -10, -i * riverLength));
	}
	boatSpeed = 300;
	healthPoints = 100;
	score = 1;
	fg->clear();
	isPlay = true;
}

void Scene::update(float deltaTime, Matrix4f viewMatrix)
{
	boat.updateUniformVariable("ViewMatrix", viewMatrix);
	scenery.updateUniformVariable("ViewMatrix", viewMatrix);
	river.updateUniformVariable("ViewMatrix", viewMatrix);
	river.updateUniformVariable("SpeedFactor", boatSpeed/100);
	coconut.updateUniformVariable("ViewMatrix", viewMatrix);
	monkey.updateUniformVariable("ViewMatrix", viewMatrix);
	croc.updateUniformVariable("ViewMatrix", viewMatrix);

	if(!isPlay)
		return;

	animateMonkeys(deltaTime);
	animateCoconut(deltaTime);
	animateCrocodile(deltaTime);
	animateScenery(deltaTime);
	checkCollisions(deltaTime);

	if(boatSpeed < 500)
	{
		boatSpeed += 5 * deltaTime;
		if(boatSpeed > 500)
			boatSpeed= 500;
	}

	if(healthPoints <= 0)
	{
		isPlay = false;
		score = TimeManager::elapsedTime() - levelStartTime;
		startBlock = TimeManager::elapsedTime();
		displayForeground();
	}
}

NodeTransitPtr Scene::getBase()
{
	return NodeTransitPtr(base);
}

void Scene::animateMonkeys(float deltaTime)
{
	for(int i = 0; i < monkeys.size(); i++)
	{
		if(monkeys[i].monkeyState != Monkey::State::Inactive && (monkeys[i].getTranslation().y() < -150 || monkeys[i].getTranslation().z() >= 300))
		{
			monkeys[i].setTranslation(Vec3f(0, 0, 1000));
			monkeys[i].setRotation(Quaternion::identity());
			monkeys[i].monkeyState = Monkey::State::Inactive;
			continue;
		}

		float xPos = 0;
		float jumpProgress = 0;
		float jumpSpeed = 200;
		switch(monkeys[i].monkeyState)
		{
		case Monkey::State::Hit:
			monkeys[i].Velocity += Vec3f(0,-gravity*deltaTime,0);
			monkeys[i].setTranslation(monkeys[i].getTranslation() + monkeys[i].Velocity * deltaTime);
			break;
		case Monkey::State::OnRock:
			monkeys[i].setTranslation(Vec3f(monkeys[i].getTranslation().x(), monkeys[i].BaseY + 10 * osgPow(sin(TimeManager::elapsedTime()*20), 2), monkeys[i].getTranslation().z() + boatSpeed * deltaTime));
			if(monkeys[i].getTranslation().z() >= -300)
			{
				/* Get jump interpolation deltaX based on distance to landing spot */
				float spotsWidth = boatWidth - 20;
				monkeys[i].jumpTarget = Vec3f((spotsWidth/(monkeys.size()-1) * i - spotsWidth/2 ), 0, -75);
				monkeys[i].jumpDirection = monkeys[i].jumpTarget - monkeys[i].getTranslation();
				monkeys[i].jumpDirection.normalize();
				monkeys[i].jumpDeltaX = osgAbs(monkeys[i].getTranslation().x() - monkeys[i].jumpTarget.x());
				monkeys[i].monkeyState = Monkey::State::Jumping;
			}
			break;
		case Monkey::State::OnBoat:
			healthPoints -= 1 * deltaTime;
			monkeys[i].setTranslation(Vec3f(monkeys[i].getTranslation().x(), monkeys[i].BaseY + 10 * osgPow(sin(TimeManager::elapsedTime()*20), 2), monkeys[i].getTranslation().z()));
			break;
		case Monkey::State::Jumping:
			xPos = monkeys[i].getTranslation().x() + monkeys[i].jumpDirection.x() * jumpSpeed * deltaTime;
			jumpProgress = 1 - ((i % 2 == 0 ? monkeys[i].jumpTarget.x() - xPos : xPos - monkeys[i].jumpTarget.x())/monkeys[i].jumpDeltaX);

			monkeys[i].setTranslation(
				Vec3f(xPos,
				(i % 2 == 0 ? leftRock.y() : rightRock.y()) +  100 * sin(jumpProgress * Pi * 1.3f),
				monkeys[i].getTranslation().z() + monkeys[i].jumpDirection.z() * jumpSpeed * deltaTime));

			if(jumpProgress >= 1.f)
			{
				monkeys[i].setTranslation(monkeys[i].jumpTarget);
				monkeys[i].BaseY = 0;
				monkeys[i].monkeyState = Monkey::State::OnBoat;
			}
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
		{
			riverSections[i].setTranslation(riverSections[i].getTranslation() + Vec3f(0,0,-((int)riverSections.size()) * riverLength));

			/* Set croc in position */
			crocodiles[i].setTranslation(Vec3f(osgRand() * boatWidth - boatWidth/2, 0, riverSections[i].getTranslation().z()));
			crocodiles[i].crocState = Crocodile::State::Lurking;

			/* Try to spawn a monkey with 50% chance */
			if(osgRand() <= .5f)
			{
				std::cout << "Spawning monkey" << std::endl;
				for(int j = 0; j < monkeys.size(); j++)
				{
					if(monkeys[j].monkeyState != Monkey::State::Inactive)
					{
						std::cout << "Monkey " << j << " not available" << std::endl;
						continue;
					}

					std::cout << "Monkey " << j << " spawned with success" << std::endl;
					monkeys[j].monkeyState = Monkey::State::OnRock;
					monkeys[j].setTranslation(riverSections[i].getTranslation() + (j % 2 == 0 ? leftRock : rightRock));
					monkeys[j].BaseY = monkeys[j].getTranslation().y();
					monkeys[j].setRotation(Quaternion(Vec3f(0,1,0), osgDegree2Rad(j % 2 == 0 ? 30 : -30)));
					break;
				}
			}
		}
	}
}

void Scene::animateCrocodile(float deltaTime)
{
	for(int i = 0; i < crocodiles.size(); i++)
	{
		switch(crocodiles[i].crocState)
		{
		case Crocodile::State::Hit:
			crocodiles[i].setTranslation(crocodiles[i].getTranslation() + Vec3f(0, -30 * boatSpeed/100 * deltaTime, 0));
			if(crocodiles[i].getTranslation().y() < -50)
			{
				crocodiles[i].setTranslation(Vec3f(0, 0, 1000));
				crocodiles[i].crocState = Crocodile::State::Inactive;
			}
			break;
		case Crocodile::State::Lurking:
			crocodiles[i].setTranslation(crocodiles[i].getTranslation() + boatSpeed * deltaTime * Vec3f(0,0,1));
			if(crocodiles[i].getTranslation().z() + crocodiles[i].getBoxDimensions().z()/2 > -boatLength/2)
			{
				std::cout << "Croc damaged raft for 15 HP. Remaining: " << healthPoints << " HP" << std::endl;
				healthPoints -= 15;
				crocodiles[i].crocState = Crocodile::State::Hit;
			}
			break;
		case Crocodile::State::Inactive:
			break;
		}
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

	/* Test coconut against all crocodiles */
	for(int i = 0; i < crocodiles.size(); i++)
	{
		if(crocodiles[i].crocState != Crocodile::State::Lurking)
			continue;

		if(crocodileCoconutIntersection(&crocodiles[i]))
		{
			std::cout << "Croc hit!" << std::endl;
			Vec3f temp = crocodiles[i].getTranslation() - coconut.getTranslation();
			crocodiles[i].crocState = Crocodile::State::Hit;
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
	if(isCoconutActive || !isPlay)
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

/* Source: https://stackoverflow.com/questions/4578967/cube-sphere-intersection-test */
bool Scene::crocodileCoconutIntersection(Crocodile* croc)
{
	Vec3f C1 = croc->getTranslation() - croc->getBoxDimensions()/2;
	Vec3f C2 = croc->getTranslation() + croc->getBoxDimensions()/2;
	Vec3f S = coconut.getTranslation();
	float R = coconutRadius;
    float dist_squared = R * R;

    /* assume C1 and C2 are element-wise sorted, if not, do that now */
    if (S.x() < C1.x()) dist_squared -= osgPow((S.x() - C1.x()),2);
    else if (S.x() > C2.x()) dist_squared -= osgPow((S.x() - C2.x()),2);
    if (S.y() < C1.y()) dist_squared -= osgPow((S.y() - C1.y()),2);
    else if (S.y() > C2.y()) dist_squared -= osgPow((S.y() - C2.y()),2);
    if (S.z() < C1.z()) dist_squared -= osgPow((S.z() - C1.z()),2);
    else if (S.z() > C2.z()) dist_squared -= osgPow((S.z() - C2.z()),2);
    return dist_squared > 0;
}

float Scene::getScore()
{
	return score;
}

void Scene::displayForeground()
{
	fg->clearImages();
	if(!isPlay)
	{
		fg->addImage(startImg, Pnt2f(windowX/2 - 200, windowY/2 - 60));
		if(score == 0)
			return;
		if(score > surviveTime)
			fg->addImage(winImg, Pnt2f(windowX/2 - 200, windowY/2 + 60));
		else
			fg->addImage(loseImg, Pnt2f(windowX/2 - 200, windowY/2 + 60));
	}
}

ImageForegroundRecPtr Scene::getImageForeground()
{
	return fg;
}