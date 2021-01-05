#pragma once
#ifndef _POSTPROCESSGRAPH_H_
#define _POSTPROCESSGRAPH_H_

#include "PostProcessPass.h"

class PostProcessGraph : public Serializable
{
public:
	Serialize(PostProcessGraph);

	PostProcessResource resource;

	list<PostProcessPass*> passes;

	~PostProcessGraph();

	void addPostProcessPass(PostProcessPass& pass);
	void addDefaultPasses();

	virtual void render(RenderInfo& info);
	virtual void resize(Unit2Di size);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_POSTPROCESSGRAPH_H_
