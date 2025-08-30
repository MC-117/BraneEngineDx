#pragma once

#include "../TextureCube.h"

class Material;

class TextureCubePool
{
public:
	struct Slot;
	typedef shared_ptr<Slot> SlotPtr;
	struct Slot
	{
		SlotPtr lastEmpty;
		SlotPtr nextEmpty;
		int index = -1;
		TextureCube* cubeMap = NULL;
	};
	unordered_map<TextureCube*, SlotPtr> cubeMapIndex;
	vector<SlotPtr> slots;
	unordered_set<TextureCube*> refreshMapes;
	SlotPtr firstEmptySlot;
	SlotPtr endEmptySlot;
	int width = 128;
	TextureCube cubeMapArray;

	TextureCubePool();

	void reset(int size);
	int reuseCubeMap(TextureCube* cubeMap);
	int getCubeMapIndex(TextureCube* cubeMap) const;
	bool setCubeMap(const vector<TextureCube*>& cubeMaps);
	bool markRefreshCubeMap(TextureCube* cubeMap);
	void prepare();
	void refreshCubePool(IRenderContext& context);
protected:
	static Material* copyMaterial;
	static ShaderProgram* copyProgram;
	static ComputePipelineState* copyPSO;
	static Material* genMipsMaterial;
	static ShaderProgram* genMipsProgram;
	static ComputePipelineState* genMipsPSO;
	static bool isInited;

	static void loadDefaultResource();
};