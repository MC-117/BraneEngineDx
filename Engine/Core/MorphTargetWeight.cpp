#include "MorphTargetWeight.h"

MorphTargetWeight::~MorphTargetWeight()
{
	if (renderData) {
		renderData->release();
		delete renderData;
	}
}

void MorphTargetWeight::setMesh(Mesh& mesh)
{
	SkeletonMesh* skeletonMesh = dynamic_cast<SkeletonMesh*>(&mesh);
	this->mesh = skeletonMesh;
	if (skeletonMesh != NULL) {
		int size = skeletonMesh->morphName.size();
		if (size > 0) {
			morphWeights.resize(size + 1, 0);
			morphWeights[0] = size;
		}
		else
			morphUpdate = false;
	}
}

int MorphTargetWeight::getMorphCount() const
{
	return max(0llu, morphWeights.size() - 1);
}

bool MorphTargetWeight::getMorphWeight(unsigned int index, float& weight)
{
	index++;
	if (index < morphWeights.size()) {
		weight = morphWeights[index];
		return true;
	}
	return false;
}

bool MorphTargetWeight::setMorphWeight(unsigned int index, float weight)
{
	index++;
	if (index < morphWeights.size()) {
		morphWeights[index] = weight;
		morphUpdate = true;
		return true;
	}
	return false;
}

IRenderData* MorphTargetWeight::getRenderData()
{
	if (renderData)
		return renderData;
	MorphTargetWeightRenderData* weightRenderData = new MorphTargetWeightRenderData();
	weightRenderData->source = this;
	renderData = weightRenderData;
	return renderData;
}

void MorphTargetWeightRenderData::create()
{
	if (source == NULL)
		return;
	needUpdate = source->morphUpdate;
	if (needUpdate) {
		morphWeights = source->morphWeights;
		source->morphUpdate = false;
	}
}

void MorphTargetWeightRenderData::release()
{
	weightGPUBuffer.resize(0);
}

void MorphTargetWeightRenderData::upload()
{
	if (needUpdate && !morphWeights.empty()) {
		weightGPUBuffer.resize(morphWeights.size());
		weightGPUBuffer.uploadData(morphWeights.size(), morphWeights.data());
		needUpdate = false;
	}
}

void MorphTargetWeightRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName morphWeightName = "morphWeight";
	context.bindBufferBase(weightGPUBuffer.getVendorGPUBuffer(), morphWeightName); // MORPHWEIGHT_BIND_INDEX
}

void MorphTargetRemapper::addMorphTargetWeight(MorphTargetWeight& weights)
{
	SkeletonMesh* skeletonMesh = dynamic_cast<SkeletonMesh*>(weights.mesh);
	if (skeletonMesh == NULL)
		return;
	int size = skeletonMesh->morphName.size();
	for (int i = 0; i < size; i++) {
		string& name = skeletonMesh->morphName[i];
		auto iter = morphNameToIndex.find(name);
		if (iter == morphNameToIndex.end()) {
			morphNameToIndex.insert(make_pair(name, morphWeights.size()));
			morphNames.push_back(name);
			morphWeights.emplace_back().push_back({ &weights, i });
		}
		else {
			morphWeights[iter->second].push_back({ &weights, i });
		}
	}
}

void MorphTargetRemapper::clear()
{
	morphNameToIndex.clear();
	morphNames.clear();
	morphWeights.clear();
	morphChannelMap.clear();
}

int MorphTargetRemapper::getMorphCount() const
{
	return morphWeights.size();
}

bool MorphTargetRemapper::getMorphWeight(unsigned int index, float& weight)
{
	if (index < morphWeights.size()) {
		auto info = morphWeights[index];
		for (int i = 0; i < info.size(); i++) {
			MappedInfo& mi = info[i];
			mi.weights->getMorphWeight(mi.index, weight);
		}
		return true;
	}
	return false;
}

bool MorphTargetRemapper::setMorphWeight(unsigned int index, float weight)
{
	if (index < morphWeights.size()) {
		auto info = morphWeights[index];
		for (int i = 0; i < info.size(); i++) {
			MappedInfo& mi = info[i];
			mi.weights->setMorphWeight(mi.index, weight);
		}
		return true;
	}
	return false;
}

void MorphTargetRemapper::updateMorphWeight(const map<unsigned int, float>& weightMap)
{
	auto end = weightMap.end();
	for (int i = 0; i < morphChannelMap.size(); i++) {
		int index = morphChannelMap[i];
		if (index >= 0) {
			auto iter = weightMap.find(index);
			if (iter != end)
				setMorphWeight(i, iter->second);
		}
	}
}
