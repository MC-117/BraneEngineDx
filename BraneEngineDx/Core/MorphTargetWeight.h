#pragma once

#include "GPUBuffer.h"
#include "SkeletonMesh.h"
#include "RenderCore/RenderInterface.h"

class MorphTargetWeight
{
public:
	Mesh* mesh = NULL;
	vector<float> morphWeights;
	bool morphUpdate = true;

	virtual ~MorphTargetWeight();
	void setMesh(Mesh& mesh);

	int getMorphCount() const;
	bool getMorphWeight(unsigned int index, float& weight);
	bool setMorphWeight(unsigned int index, float weight);

	IRenderData* getRenderData();
protected:
	IRenderData* renderData = NULL;
};

struct MorphTargetWeightRenderData : public IRenderData
{
	MorphTargetWeight* source = NULL;
	vector<float> morphWeights;
	bool needUpdate = false;
	GPUBuffer weightGPUBuffer = GPUBuffer(GB_Storage, GBF_Float);

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};

class MorphTargetRemapper
{
public:
	struct MappedInfo
	{
		MorphTargetWeight* weights;
		int index;
	};
	map<string, int> morphNameToIndex;
	vector<string> morphNames;
	vector<vector<MappedInfo>> morphWeights;

	vector<int> morphChannelMap;

	void addMorphTargetWeight(MorphTargetWeight& weights);
	void clear();

	int getMorphCount() const;
	bool getMorphWeight(unsigned int index, float& weight);
	bool setMorphWeight(unsigned int index, float weight);

	void updateMorphWeight(const map<unsigned int, float>& weightMap);
};