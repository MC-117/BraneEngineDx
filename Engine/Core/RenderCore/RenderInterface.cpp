#include "RenderInterface.h"

#include "MaterialRenderData.h"
#include "RenderCoreUtility.h"
#include "../IVendor.h"
#include "../Material.h"
#include "../Utility/Parallel.h"

void IRenderData::updateMainThread()
{
	create();
}

void IRenderData::updateRenderThread()
{
	upload();
}

MaterialDrawData::MaterialDrawData(Material* material)
{
	if (material && !material->isNull()) {
		shader = material->getShader();
		renderData = material->getRenderData();
		renderOrder = material->getRenderOrder();
		canCastShadow = material->canCastShadow;
		drawKey = (size_t)material;
	}
}

bool MaterialDrawData::isValid() const
{
	return shader && renderData;
}

void IRenderCommand::collectRenderData(IRenderDataCollector* collectorMainThread, IRenderDataCollector* collectorRenderThread)
{
	if (collectorMainThread) {
		unsigned long long frames = Time::frames();
		if (materialRenderData) {
			updateRenderDataMainThread(materialRenderData, frames);
		}
		for (auto binding : bindings) {
			if (binding) {
				updateRenderDataMainThread(binding, frames);
			}
		}
	}

	if (collectorRenderThread) {
		if (materialRenderData) {
			collectorRenderThread->add(*materialRenderData);
		}
		for (auto binding : bindings) {
			if (binding) {
				collectorRenderThread->add(*binding);
			}
		}
	}
}

IRenderPack::~IRenderPack()
{
	
}

void BaseRenderDataCollector::add(IUpdateableRenderData& data)
{
	collection.insert(&data);
}

void BaseRenderDataCollector::clear()
{
	collection.clear();
}

void BaseRenderDataCollector::updateMainThread(long long mainFrame)
{
	for (auto data : collection) {
		updateRenderDataMainThread(data, mainFrame);
	}
}

void BaseRenderDataCollector::updateRenderThread(long long renderFrame)
{
	for (auto data : collection) {
		updateRenderDataRenderThread(data, renderFrame);
	}
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
