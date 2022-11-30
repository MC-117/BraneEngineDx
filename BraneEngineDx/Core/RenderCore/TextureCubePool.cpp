#include "TextureCubePool.h"
#include "../Asset.h"
#include "../Material.h"

TextureCubePool::TextureCubePool() : cubeMapArray(128, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear, TIT_RGBA8_UF })
{
	cubeMapArray.setViewAsArray(true);
	cubeMapArray.setAutoGenMip(false);
}

void TextureCubePool::reset(int size)
{
	if (slots.size() != size) {
		slots.resize(size);
		cubeMapIndex.clear();
	}
	if (slots.empty())
		return;
	cubeMapArray.resize(width, width, size);
	endEmptySlot = NULL;
	int i = 0;
	for (auto& slot : slots) {
		if (slot == NULL)
			slot = make_shared<Slot>();
		slot->lastEmpty = endEmptySlot;
		slot->index = i;
		if (endEmptySlot)
			endEmptySlot->nextEmpty = slot;
		endEmptySlot = slot;
		i++;
	}
	firstEmptySlot = slots.front();
}

int TextureCubePool::reuseCubeMap(TextureCube* cubeMap)
{
	auto iter = cubeMapIndex.find(cubeMap);
	if (iter == cubeMapIndex.end())
		return -1;
	iter->second->cubeMap = cubeMap;
	return iter->second->index;
}

int TextureCubePool::getCubeMapIndex(TextureCube* cubeMap) const
{
	auto iter = cubeMapIndex.find(cubeMap);
	if (iter == cubeMapIndex.end())
		return -1;
	return iter->second->index;
}

bool TextureCubePool::setCubeMap(const vector<TextureCube*>& cubeMaps)
{
	reset(cubeMaps.size());
	vector<TextureCube*> newCubeMaps;
	for (TextureCube* cubeMap : cubeMaps) {
		if (reuseCubeMap(cubeMap) < 0)
			newCubeMaps.push_back(cubeMap);
	}
	for (TextureCube* cubeMap : newCubeMaps) {
		SlotPtr slot = firstEmptySlot;
		if (slot->cubeMap)
			cubeMapIndex.erase(slot->cubeMap);
		slot->cubeMap = cubeMap;
		cubeMapIndex[cubeMap] = slot;
		firstEmptySlot = slot->nextEmpty;
		slot->lastEmpty = NULL;
		slot->nextEmpty = NULL;
		refreshMapes.insert(cubeMap);
	}
	if (firstEmptySlot)
		firstEmptySlot->lastEmpty = NULL;
	else
		endEmptySlot = NULL;
	return true;
}

bool TextureCubePool::markRefreshCubeMap(TextureCube* cubeMap)
{
	refreshMapes.insert(cubeMap);
	return true;
}

void TextureCubePool::prepare()
{
	if (refreshMapes.empty())
		return;
	loadDefaultResource();
	program->init();
}

void TextureCubePool::refreshCubePool(IRenderContext& context)
{
	if (refreshMapes.empty())
		return;
	context.bindShaderProgram(program);
	Image image;
	image.texture = &cubeMapArray;
	image.arrayCount = 6;
	MipOption mipOption;
	mipOption.dimension = TD_Array;
	mipOption.arrayCount = 6;
	Vector3u localSize = material->getLocalSize();
	localSize.x() = ceilf(cubeMapArray.getWidth() / (float)localSize.x());
	localSize.y() = ceilf(cubeMapArray.getHeight() / (float)localSize.y());
	localSize.z() = ceilf(6 / (float)localSize.z());
	for (auto& cubeMap : refreshMapes) {
		int index = getCubeMapIndex(cubeMap);
		if (index < 0)
			continue;
		image.arrayBase = index * 6;
		context.bindTexture((ITexture*)cubeMap->getVendorTexture(), "srcTex", mipOption);
		context.bindImage(image, "dstTex");
		context.dispatchCompute(localSize.x(), localSize.y(), localSize.z());
	}
	context.bindTexture(NULL, "srcTex");
	image.texture = NULL;
	context.bindImage(image, "dstTex");
	refreshMapes.clear();
}

Material* TextureCubePool::material = NULL;
ShaderProgram* TextureCubePool::program = NULL;
bool TextureCubePool::isInited = false;

void TextureCubePool::loadDefaultResource()
{
	if (isInited)
		return;
	material = getAssetByPath<Material>("Engine/Shaders/Pipeline/CopyTex2DArray.mat");
	if (material == NULL)
		throw runtime_error("Not found default shader");
	program = material->getShader()->getProgram(Shader_Default);
	if (program == NULL)
		throw runtime_error("Not found default shader");
	isInited = true;
}
