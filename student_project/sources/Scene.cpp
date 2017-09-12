#include "Scene.h"

Scene::Scene()
{	riverSections = std::vector<GameObject>();

	riverLength = 1000;
	boatSpeed = 100;
}


Scene::~Scene()
{
}

void Scene::initialize()
{
	/* Main Directional Light */
	DirectionalLightRecPtr dirLight = DirectionalLight::create();
	dirLight->setDirection(1,1,-1);
	dirLight->setDiffuse(Color4f(1,1,1,1));
	dirLight->setAmbient(Color4f(.2f,.2f,.2f,1));
	dirLight->setSpecular(Color4f(1,1,1,1));
	
	/*Lit Scene*/
	NodeRecPtr world = Node::create();
	world->setCore(Group::create());

	/* Load river sections */
	NodeRecPtr river = SceneFileHandler::the()->read("models/river.3ds");
	for(int i = 0; i < 4; i++)
	{
		ComponentTransformRecPtr trans = ComponentTransform::create();
		trans->setTranslation(Vec3f(0, 0, -i * riverLength));
		GameObject riverSection = GameObject(trans, OSG::deepCloneTree(river));
		riverSections.push_back(riverSection);
		world->addChild(riverSection.getNode());
	}
	
	/*Light Sources*/	
	NodeRecPtr lights = Node::create();
	lights->setCore(Group::create());
	
	dirLight->setBeacon(world);
	base = makeNodeFor(dirLight);
	base->addChild(world);
	base->addChild(lights);
}

NodeTransitPtr Scene::getBase()
{
	return NodeTransitPtr(base);
}

void Scene::update(float deltaTime)
{
	for each (GameObject go in riverSections)
	{
		go.setTranslation(go.getTranslation() + boatSpeed * deltaTime * Vec3f(0,0,1));
		if(go.getTranslation().z() > riverLength)
			go.setTranslation(go.getTranslation() + Vec3f(0,0,-((int)riverSections.size()) * riverLength));
	}
}
