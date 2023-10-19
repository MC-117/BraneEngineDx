#include "RenderInterface.h"
#include "../IVendor.h"

IRenderPack::~IRenderPack()
{
	
}

void BaseRenderDataCollector::add(IRenderData& data)
{
	collection.insert(&data);
}

void BaseRenderDataCollector::clear()
{
	collection.clear();
}

void BaseRenderDataCollector::upload()
{
	for (auto data : collection)
		data->upload();
}

SerializeInstance(RenderGraph);

void RenderGraph::getPasses(vector<pair<string, RenderPass*>>& passes)
{
}

Serializable* RenderGraph::instantiate(const SerializationInfo& from)
{
	return nullptr;
}

void RenderPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
}
