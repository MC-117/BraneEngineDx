#pragma once

#include "GPUBuffer.h"
#include "SkeletonMesh.h"

class MorphTargetWeight : public IBufferBinding
{
public:
	Mesh* mesh = NULL;
	vector<float> morphWeights;
	GPUBuffer weightGPUBuffer = GPUBuffer(GPUBufferType::GB_Storage, sizeof(float));
	bool morphUpdate = true;

	void setMesh(Mesh& mesh);

	int getMorphCount() const;
	bool getMorphWeight(unsigned int index, float& weight);
	bool setMorphWeight(unsigned int index, float weight);

	void updateBuffer();
	virtual void bindBuffer();
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