#include "PostProcessGraph.h"
#include "SSAOPass.h"
#include "BloomPass.h"
#include "ToneMapPass.h"
#include "BlurPass.h"
#include "BlitPass.h"
#include "DOFPass.h"
#include "VolumetricLightPass.h"

SerializeInstance(PostProcessGraph);

PostProcessGraph::~PostProcessGraph()
{
	for (auto b = passes.begin(), e = passes.end(); b != e; b++) {
		delete *b;
	}
}

void PostProcessGraph::addPostProcessPass(PostProcessPass & pass)
{
	passes.push_back(&pass);
	pass.setResource(resource);
	pass.setEnable(true);
}

void PostProcessGraph::removePostProcessPass(const string& name)
{
	for (auto b = passes.begin(), e = passes.end(); b != e; b++) {
		if ((*b)->getName() == name) {
			delete *b;
			passes.erase(b);
			break;
		}
	}
}

void PostProcessGraph::addDefaultPasses()
{
	addPostProcessPass(*new SSAOPass());
	addPostProcessPass(*new DOFPass());
	addPostProcessPass(*new BloomPass());
	addPostProcessPass(*new VolumetricLightPass());
	addPostProcessPass(*new ToneMapPass());
	//addPostProcessPass(*new BlurPass());
	addPostProcessPass(*new BlitPass());
}

PostProcessPass* PostProcessGraph::getPostProcessPass(const string& name)
{
	for (auto b = passes.begin(), e = passes.end(); b != e; b++) {
		if ((*b)->getName() == name)
			return *b;
	}
	return NULL;
}

void PostProcessGraph::render(RenderInfo & info)
{
	for (auto b = passes.begin(), e = passes.end(); b != e; b++) {
		(*b)->render(info);
	}
}

void PostProcessGraph::resize(Unit2Di size)
{
	for (auto b = passes.begin(), e = passes.end(); b != e; b++) {
		(*b)->resize(size);
	}
	if (resource.finalRenderTarget != NULL)
		resource.finalRenderTarget->resize(size.x, size.y);
}

Serializable * PostProcessGraph::instantiate(const SerializationInfo & from)
{
	return new PostProcessGraph();
}

bool PostProcessGraph::deserialize(const SerializationInfo & from)
{
	for (auto b = passes.begin(), e = passes.end(); b != e; b++) {
		if (!isClassOf<BlitPass>(*b))
			from.get((*b)->getName(), *(PostProcessPass*)(*b));
	}
	return true;
}

bool PostProcessGraph::serialize(SerializationInfo & to)
{
	Serializable::serialize(to);
	for (auto b = passes.begin(), e = passes.end(); b != e; b++) {
		if (!isClassOf<BlitPass>(*b))
			to.set((*b)->getName(), **b);
	}
	return true;
}