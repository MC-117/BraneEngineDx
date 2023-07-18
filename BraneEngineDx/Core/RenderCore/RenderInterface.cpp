#include "RenderInterface.h"
#include "../IVendor.h"

IRenderPack::~IRenderPack()
{
	if (vendorRenderExecution != NULL)
		delete vendorRenderExecution;
}

void IRenderPack::newVendorRenderExecution()
{
	if (vendorRenderExecution == NULL) {
		vendorRenderExecution = VendorManager::getInstance().getVendor().newRenderExecution();
		if (vendorRenderExecution == NULL) {
			throw runtime_error("Vendor new RenderExecution failed");
		}
	}
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
