#pragma once

#include <string>

#include <OpenSG/OSGComponentTransform.h>
#include <OpenSG\OSGChunkMaterial.h>
#include <OpenSG\OSGTextureObjChunk.h>
#include <OpenSG\OSGMaterialGroup.h>
#include <OpenSG\OSGSimpleSHLChunk.h>
#include <OpenSG\OSGImage.h>

#include "..\shader\Shaders.h"

OSG_USING_NAMESPACE

class GameObject
{
private:
	NodeRecPtr geometry;
	NodeRecPtr transform;
	NodeRecPtr materialNode;
	SimpleSHLChunkRecPtr shl;
	ChunkMaterialRecPtr cmat;
	TextureObjChunkRecPtr tex;
	MaterialGroupRecPtr material;

	void init(ComponentTransformRecPtr transform, NodeRecPtr geometry, std::string textureImage)
	{
		this->transform = makeNodeFor(transform);
		this->geometry = geometry;

		cmat = ChunkMaterial::create();
		material = MaterialGroup::create();
		tex = TextureObjChunk::create();
		shl = SimpleSHLChunk::create();
		ImageRecPtr img = Image::create();
		
		shl->setFragmentProgram(_fragment_shader);
		shl->setVertexProgram(_vertex_shader);
		shl->addUniformVariable("ViewMatrix", Matrix4f());
		img->read(textureImage.c_str());
		img->removeMipmap();		
		tex->setImage(img);
		tex->setMinFilter(GL_LINEAR);
		tex->setMagFilter(GL_LINEAR);
		cmat->addChunk(tex);
		cmat->addChunk(shl);
		
		material->setMaterial(cmat);
		materialNode = makeNodeFor(this->material);
		materialNode->addChild(this->geometry);
		this->transform->addChild(materialNode);
	}

public:
	GameObject(){}

	GameObject(ComponentTransformRecPtr transform, NodeRecPtr geometry, std::string textureImage)
	{
		init(transform, geometry, textureImage);
	}
	GameObject(NodeRecPtr geometry, std::string textureImage)
	{
		ComponentTransformRecPtr transform = ComponentTransform::create();
		transform->setTranslation(Vec3f(0,0,0));
		init(transform, geometry, textureImage);
	}
	GameObject(Vec3f position, NodeRecPtr geometry, std::string textureImage)
	{
		ComponentTransformRecPtr transform =  ComponentTransform::create();
		transform->setTranslation(position);
		init(transform, geometry, textureImage);
	}
	GameObject(Vec3f position, Quaternion rotation, NodeRecPtr geometry, std::string textureImage)
	{
		ComponentTransformRecPtr transform =  ComponentTransform::create();
		transform->setRotation(rotation);
		transform->setTranslation(position);
		init(transform, geometry, textureImage);
	}

	~GameObject(){}

	void setTranslation(Vec3f pos)
	{
		dynamic_cast<ComponentTransform*>(transform->getCore())->setTranslation(pos);
	}
	void setRotation(Quaternion quat)
	{
		dynamic_cast<ComponentTransform*>(transform->getCore())->setRotation(quat);
	}

	Vec3f getTranslation()
	{
		return dynamic_cast<ComponentTransform*>(transform->getCore())->getTranslation();
	}

	Quaternion getRotation()
	{
		return dynamic_cast<ComponentTransform*>(transform->getCore())->getRotation();
	}

	NodeRecPtr getNode()
	{
		return transform;
	}

	NodeRecPtr getMaterialNode()
	{
		return materialNode;
	}

	void setMaterialNode(NodeRecPtr materialNode)
	{
		this->materialNode = materialNode;
	}

	NodeRecPtr getGeometryNode()
	{
		return geometry;
	}

	void setGeomtryNode(NodeRecPtr geometryNode)
	{
		this->geometry = geometryNode;
	}

	void addChild(NodeRecPtr node)
	{
		transform->addChild(node);
	}

	void setVertexProgram(std::string program)
	{
		shl->setVertexProgram(program);
	}

	template<class ValueT>
	void addUniformVariable(std::string name, const ValueT &value)
	{
		shl->addUniformVariable(name.c_str(), value);
	}

	template<class ValueT>
	void updateUniformVariable(std::string name, const ValueT &value)
	{
		shl->updateUniformVariable(name.c_str(), value);
	}

	SimpleSHLChunkRecPtr getSHLChunk()
	{
		return shl;
	}

	void setSHLChunk(SimpleSHLChunkRecPtr shl)
	{
		this->shl = shl;
	}
};