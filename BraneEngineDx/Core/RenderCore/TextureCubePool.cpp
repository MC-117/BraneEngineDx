#include "TextureCubePool.h"
#include "../Asset.h"
#include "../Material.h"
#include "../Profile/ProfileCore.h"

TextureCubePool::TextureCubePool() : cubeMapArray(128, 4, false, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Linear, TF_Linear_Mip_Linear, TIT_RGBA8_UF })
{
	cubeMapArray.setViewAsArray(true);
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
	copyProgram->init();
	genMipsProgram->init();
}

void TextureCubePool::refreshCubePool(IRenderContext& context)
{
	if (refreshMapes.empty())
		return;

	static const ShaderPropertyName srcTexName = "srcTex";
	static const ShaderPropertyName dstTexName = "dstTex";
	static const ShaderPropertyName srcColorName = "srcColor";
	static const ShaderPropertyName dstColorName = "dstColor";

	Image image;
	image.texture = &cubeMapArray;
	image.arrayCount = 6;
	MipOption mipOption;
	mipOption.dimension = TD_Array;
	mipOption.arrayCount = 6;
	mipOption.mipCount = 1;

	// Copy
	Vector3u localSize = copyMaterial->getLocalSize();
	Vector3u dispatchSize = Vector3u(
		ceilf(cubeMapArray.getWidth() / (float)localSize.x()),
		ceilf(cubeMapArray.getHeight() / (float)localSize.y()),
		ceilf(6 / (float)localSize.z())
	);

	context.bindShaderProgram(copyProgram);
	for (auto& cubeMap : refreshMapes) {
		int index = getCubeMapIndex(cubeMap);
		if (index < 0)
			continue;
		image.arrayBase = index * 6;
		context.bindTexture((ITexture*)cubeMap->getVendorTexture(), srcTexName, mipOption);
		context.bindImage(image, dstTexName);
		context.dispatchCompute(dispatchSize.x(), dispatchSize.y(), dispatchSize.z());
	}
	context.unbindBufferBase(srcTexName);
	context.unbindBufferBase(dstTexName);

	// GenMips
	int mipLevelsToProcess = cubeMapArray.getMipLevels() - 1;
	if (mipLevelsToProcess > 0) {
		context.bindShaderProgram(genMipsProgram);
		for (auto& cubeMap : refreshMapes) {
			int index = getCubeMapIndex(cubeMap);
			if (index < 0)
				continue;
			image.arrayBase = index * 6;
			mipOption.arrayBase = index * 6;
			int mipWidth = cubeMapArray.getWidth();
			int mipHeight = cubeMapArray.getHeight();
			for (int mipIndex = 0; mipIndex < mipLevelsToProcess; mipIndex++, mipWidth >>= 1, mipHeight >>= 1) {
				image.level = mipIndex + 1;
				mipOption.detailMip = mipIndex;
				context.bindTexture((ITexture*)cubeMapArray.getVendorTexture(), srcColorName, mipOption);
				context.bindImage(image, dstColorName);
				dispatchSize.x() = ceilf(mipWidth / (float)localSize.x());
				dispatchSize.y() = ceilf(mipHeight / (float)localSize.y());
				context.dispatchCompute(dispatchSize.x(), dispatchSize.y(), dispatchSize.z());
				context.unbindBufferBase(srcColorName);
				context.unbindBufferBase(dstColorName);
			}
		}
	}

	refreshMapes.clear();
}

Material* TextureCubePool::copyMaterial = NULL;
ShaderProgram* TextureCubePool::copyProgram = NULL;
Material* TextureCubePool::genMipsMaterial = NULL;
ShaderProgram* TextureCubePool::genMipsProgram = NULL;
bool TextureCubePool::isInited = false;

void TextureCubePool::loadDefaultResource()
{
	if (isInited)
		return;
	copyMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/CopyTex2DArray.mat");
	if (copyMaterial == NULL)
		throw runtime_error("Not found default shader");
	copyProgram = copyMaterial->getShader()->getProgram(Shader_Default);
	if (copyProgram == NULL)
		throw runtime_error("Not found default shader");
	genMipsMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/GenMipsArray.mat");
	if (genMipsMaterial == NULL)
		throw runtime_error("Not found default shader");
	genMipsProgram = genMipsMaterial->getShader()->getProgram(Shader_Default);
	if (genMipsProgram == NULL)
		throw runtime_error("Not found default shader");
	isInited = true;
}
