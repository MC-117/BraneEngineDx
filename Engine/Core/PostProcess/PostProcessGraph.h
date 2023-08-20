#pragma once
#ifndef _POSTPROCESSGRAPH_H_
#define _POSTPROCESSGRAPH_H_

#include "PostProcessPass.h"

class PostProcessGraph : public Serializable
{
public:
	Serialize(PostProcessGraph,);

	RenderTarget postRenderTarget = RenderTarget(1280, 720, 4);
	PostProcessResource resource;

	list<PostProcessPass*> passes;

	PostProcessGraph();

	virtual ~PostProcessGraph();

	void addPostProcessPass(PostProcessPass& pass);
	void removePostProcessPass(const string& name);
	void addDefaultPasses();

	virtual PostProcessPass* getPostProcessPass(const string& name);

	virtual void render(RenderInfo& info);
	virtual void resize(Unit2Di size);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_POSTPROCESSGRAPH_H_
