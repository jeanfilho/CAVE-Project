#include "Scene.h"


Scene::Scene()
{	
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

	world->addChild(makeTorus(10.f, 50.f, 32.f, 64.f));
	ComponentTransformRecPtr transform = ComponentTransform::create();
	transform->setTranslation(Vec3f(120,0,0));
	NodeTransitPtr node = makeNodeFor(transform);
	node->addChild(makeTorus(10.f, 50.f, 32.f, 64.f));
	world->addChild(node);
	
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
