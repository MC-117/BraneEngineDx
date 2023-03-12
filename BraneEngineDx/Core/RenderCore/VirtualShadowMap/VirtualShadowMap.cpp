#include "VirtualShadowMap.h"
#include "VirtualShadowMapClipmap.h"
#include "../RenderTask.h"
#include "../DirectShadowRenderPack.h"
#include "../../Utility/MathUtility.h"
#include "../../Engine.h"
#include "../../Asset.h"
#include "../../InitializationManager.h"

Vector2u VirtualShadowMapConfig::getPhysPagesXY() const
{
	const unsigned int physPagesX = physPoolWidth / VirtualShadowMapConfig::pageSize;
	const unsigned int physPagesY = ceil(maxPhysPages / float(physPagesX));
	return Vector2u(physPagesX, physPagesY);
}

VirtualShadowMapConfig& VirtualShadowMapConfig::instance()
{
	static VirtualShadowMapConfig config;
	return config;
}

Serializable* VirtualShadowMapConfig::instantiate(const SerializationInfo& from)
{
	return new VirtualShadowMapConfig();
}

bool VirtualShadowMapConfig::deserialize(const SerializationInfo& from)
{
	from.get("physPoolWidth", (int&)physPoolWidth);
	from.get("maxPhysPages", (int&)maxPhysPages);
	from.get("firstClipmapLevel", (int&)firstClipmapLevel);
	from.get("lastClipmapLevel", (int&)lastClipmapLevel);
	from.get("clipmapRadiusZScale", clipmapRadiusZScale);
	from.get("resolutionLodBiasClipmap", resolutionLodBiasClipmap);
	from.get("pageDilationBorderSizeMain", pageDilationBorderSizeMain);
	from.get("pageDilationBorderSizeLocal", pageDilationBorderSizeLocal);
	return false;
}

bool VirtualShadowMapConfig::serialize(SerializationInfo& to)
{
	serializeInit(this, to);
	to.set("physPoolWidth", (int&)physPoolWidth);
	to.set("maxPhysPages", (int&)maxPhysPages);
	to.set("firstClipmapLevel", (int&)firstClipmapLevel);
	to.set("lastClipmapLevel", (int&)lastClipmapLevel);
	to.set("clipmapRadiusZScale", clipmapRadiusZScale);
	to.set("resolutionLodBiasClipmap", resolutionLodBiasClipmap);
	to.set("pageDilationBorderSizeMain", pageDilationBorderSizeMain);
	to.set("pageDilationBorderSizeLocal", pageDilationBorderSizeLocal);
	return false;
}

class VirtualShadowMapConfigInitializer : public Initialization
{
protected:
	static VirtualShadowMapConfigInitializer instance;
	VirtualShadowMapConfigInitializer() : Initialization(InitializeStage::BeforeEngineSetup, 0) { }

	virtual bool initialze()
	{
		Engine::engineConfig.configInfo.get("virtualShadowMap", VirtualShadowMapConfig::instance());
		return true;
	}
};

VirtualShadowMapConfigInitializer VirtualShadowMapConfigInitializer::instance;

VirtualShadowMapFrameData::~VirtualShadowMapFrameData()
{
	release();
}

void VirtualShadowMapFrameData::create()
{
	vsmInfo = new GPUBuffer(GB_Constant, GBF_Struct, sizeof(VirtualShadowMapInfo), GAF_Read, CAF_Write);
	pageTable = new GPUBuffer(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None);
	pageFlags = new GPUBuffer(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None);
	projData = new GPUBuffer(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapProjectionData), GAF_ReadWrite, CAF_None);
	pageRect = new GPUBuffer(GB_Storage, GBF_Struct, sizeof(Vector4u), GAF_ReadWrite, CAF_None);
	physPageMetaData = new GPUBuffer(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapPhysicalPageData), GAF_ReadWrite, CAF_None);
}

void VirtualShadowMapFrameData::release()
{
	if (pageTable)
		delete pageTable;
	if (pageFlags)
		delete pageFlags;
	if (projData)
		delete projData;
	if (pageRect)
		delete pageRect;
	if (physPageMetaData)
		delete physPageMetaData;
}

void VirtualShadowMapFrameData::swap(VirtualShadowMapFrameData& data)
{
	std::swap(pageTable, data.pageTable);
	std::swap(pageFlags, data.pageFlags);
	std::swap(projData, data.projData);
	std::swap(pageRect, data.pageRect);
	std::swap(physPageMetaData, data.physPageMetaData);
}

ShaderProgram* VirtualShadowMapShaders::procInvalidationsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::initPageRectsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::generatePageFlagsFromPixelsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::initPhysicalPageMetaDataProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::createCachedPageMappingsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::packFreePagesProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::allocateNewPageMappingsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::generateHierarchicalPageFlagsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::clearInitPhysPagesIndirectArgsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::selectPagesToInitProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::initPhysicalMemoryIndirectProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::cullPerPageDrawCommandsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::allocateCommandInstanceOutputSpaceProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::initOutputommandInstanceListsArgsProgram = NULL;
ShaderProgram* VirtualShadowMapShaders::outputCommandInstanceListsProgram = NULL;

Vector3u VirtualShadowMapShaders::procInvalidationsProgramDim;
Vector3u VirtualShadowMapShaders::initPageRectsProgramDim;
Vector3u VirtualShadowMapShaders::generatePageFlagsFromPixelsProgramDim;
Vector3u VirtualShadowMapShaders::initPhysicalPageMetaDataProgramDim;
Vector3u VirtualShadowMapShaders::createCachedPageMappingsProgramDim;
Vector3u VirtualShadowMapShaders::packFreePagesProgramDim;
Vector3u VirtualShadowMapShaders::allocateNewPageMappingsProgramDim;
Vector3u VirtualShadowMapShaders::generateHierarchicalPageFlagsProgramDim;
Vector3u VirtualShadowMapShaders::clearInitPhysPagesIndirectArgsProgramDim;
Vector3u VirtualShadowMapShaders::selectPagesToInitProgramDim;
Vector3u VirtualShadowMapShaders::cullPerPageDrawCommandsProgramDim;
Vector3u VirtualShadowMapShaders::allocateCommandInstanceOutputSpaceProgramDim;
Vector3u VirtualShadowMapShaders::initOutputommandInstanceListsArgsProgramDim;
Vector3u VirtualShadowMapShaders::outputCommandInstanceListsProgramDim;

const ShaderPropertyName VirtualShadowMapShaders::VSMBuffInfoName = "VSMBuffInfo";
const ShaderPropertyName VirtualShadowMapShaders::vsmPrevDataName = "vsmPrevData";
const ShaderPropertyName VirtualShadowMapShaders::pageTableName = "pageTable";
const ShaderPropertyName VirtualShadowMapShaders::pageFlagsName = "pageFlags";
const ShaderPropertyName VirtualShadowMapShaders::pageRectName = "pageRect";
const ShaderPropertyName VirtualShadowMapShaders::physPageMetaDataName = "physPageMetaData";
const ShaderPropertyName VirtualShadowMapShaders::projDataName = "projData";
const ShaderPropertyName VirtualShadowMapShaders::pageRequestFlagsName = "pageRequestFlags";
const ShaderPropertyName VirtualShadowMapShaders::physPageAllocRequestsName = "physPageAllocRequests";
const ShaderPropertyName VirtualShadowMapShaders::invalidationIndicesName = "invalidationIndices";
const ShaderPropertyName VirtualShadowMapShaders::invalidationInfoName = "invalidationInfo";
const ShaderPropertyName VirtualShadowMapShaders::outPrevPhysPageMetaDataName = "outPrevPhysPageMetaData";
const ShaderPropertyName VirtualShadowMapShaders::outPageRequestFlagsName = "outPageRequestFlags";
const ShaderPropertyName VirtualShadowMapShaders::GenPageFlagInfoName = "GenPageFlagInfo";
const ShaderPropertyName VirtualShadowMapShaders::directLightVSMIDsName = "directLightVSMIDs";
const ShaderPropertyName VirtualShadowMapShaders::sceneDepthMapName = "sceneDepthMap";
const ShaderPropertyName VirtualShadowMapShaders::sceneNormalMapName = "sceneNormalMap";
const ShaderPropertyName VirtualShadowMapShaders::prevPageTableName = "prevPageTable";
const ShaderPropertyName VirtualShadowMapShaders::prevPageFlagsName = "prevPageFlags";
const ShaderPropertyName VirtualShadowMapShaders::prevPhysPageMetaDataName = "prevPhysPageMetaData";
const ShaderPropertyName VirtualShadowMapShaders::outPageTableName = "outPageTable";
const ShaderPropertyName VirtualShadowMapShaders::outPageFlagsName = "outPageFlags";
const ShaderPropertyName VirtualShadowMapShaders::outPageRectName = "outPageRect";
const ShaderPropertyName VirtualShadowMapShaders::outPhysPageMetaDataName = "outPhysPageMetaData";
const ShaderPropertyName VirtualShadowMapShaders::outFreePhysPagesName = "outFreePhysPages";
const ShaderPropertyName VirtualShadowMapShaders::outPhysPageAllocRequestsName = "outPhysPageAllocRequests";
const ShaderPropertyName VirtualShadowMapShaders::outInitPhysPagesIndirectArgsName = "outInitPhysPagesIndirectArgs";
const ShaderPropertyName VirtualShadowMapShaders::physPagesToInitName = "physPagesToInit";
const ShaderPropertyName VirtualShadowMapShaders::outPhysPagesToInitName = "outPhysPagesToInit";

const ShaderPropertyName VirtualShadowMapShaders::physPagePoolName = "physPagePool";
const ShaderPropertyName VirtualShadowMapShaders::outPhysPagePoolName = "outPhysPagePool";

const ShaderPropertyName VirtualShadowMapShaders::CullingDataName = "CullingData";
const ShaderPropertyName VirtualShadowMapShaders::AllocCmdInfoName = "AllocCmdInfo";

const ShaderPropertyName VirtualShadowMapShaders::shadowViewInfosName = "shadowViewInfos";

const ShaderPropertyName VirtualShadowMapShaders::drawInstanceInfosName = "drawInstanceInfos";
const ShaderPropertyName VirtualShadowMapShaders::visiableInstanceInfosName = "visiableInstanceInfos";
const ShaderPropertyName VirtualShadowMapShaders::visiableInstanceCountName = "visiableInstanceCount";
const ShaderPropertyName VirtualShadowMapShaders::offsetBufferCountName = "offsetBufferCount";
const ShaderPropertyName VirtualShadowMapShaders::shadowDepthIndirectArgsName = "shadowDepthIndirectArgs";

const ShaderPropertyName VirtualShadowMapShaders::shadowDepthInstanceCounterName = "shadowDepthInstanceCounter";

const ShaderPropertyName VirtualShadowMapShaders::outDrawInstanceInfosName = "outDrawInstanceInfos";
const ShaderPropertyName VirtualShadowMapShaders::outVisiableInstanceInfosName = "outVisiableInstanceInfos";
const ShaderPropertyName VirtualShadowMapShaders::outVisiableInstanceCountName = "outVisiableInstanceCount";
const ShaderPropertyName VirtualShadowMapShaders::outOffsetBufferCountName = "outOffsetBufferCount";
const ShaderPropertyName VirtualShadowMapShaders::outShadowDepthInstanceOffsetName = "outShadowDepthInstanceOffset";

const ShaderPropertyName VirtualShadowMapShaders::outputCommandListsIndirectArgsName = "outputCommandListsIndirectArgs";
const ShaderPropertyName VirtualShadowMapShaders::outShadowDepthIndirectArgsName = "outShadowDepthIndirectArgs";

const ShaderPropertyName VirtualShadowMapShaders::pageInfoName = "pageInfo";
const ShaderPropertyName VirtualShadowMapShaders::outPageInfoName = "outPageInfo";

void VirtualShadowMapShaders::loadDefaultResource()
{
	if (procInvalidationsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_ProcInvalidations.mat");
		if (!material)
			throw runtime_error("procInvalidationsProgram load failed");
		procInvalidationsProgram = material->getShader()->getProgram(Shader_Default);
		procInvalidationsProgramDim = material->getLocalSize();
	}
	procInvalidationsProgram->init();

	if (initPageRectsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_InitPageRects.mat");
		if (!material)
			throw runtime_error("initPageRectsProgram load failed");
		initPageRectsProgram = material->getShader()->getProgram(Shader_Default);
		initPageRectsProgramDim = material->getLocalSize();
	}
	initPageRectsProgram->init();

	if (generatePageFlagsFromPixelsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_GeneratePageFlagsFromPixels.mat");
		if (!material)
			throw runtime_error("generatePageFlagsFromPixels load failed");
		generatePageFlagsFromPixelsProgram = material->getShader()->getProgram(Shader_Default);
		generatePageFlagsFromPixelsProgramDim = material->getLocalSize();
	}
	generatePageFlagsFromPixelsProgram->init();

	if (initPhysicalPageMetaDataProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_InitPhysicalPageMetaData.mat");
		if (!material)
			throw runtime_error("initPhysicalPageMetaData load failed");
		initPhysicalPageMetaDataProgram = material->getShader()->getProgram(Shader_Default);
		initPhysicalPageMetaDataProgramDim = material->getLocalSize();
	}
	initPhysicalPageMetaDataProgram->init();

	if (createCachedPageMappingsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_CreateCachedPageMappings.mat");
		if (!material)
			throw runtime_error("createCachedPageMappings load failed");
		createCachedPageMappingsProgram = material->getShader()->getProgram(Shader_Default);
		createCachedPageMappingsProgramDim = material->getLocalSize();
	}
	createCachedPageMappingsProgram->init();

	if (packFreePagesProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_PackFreePages.mat");
		if (!material)
			throw runtime_error("packFreePages load failed");
		packFreePagesProgram = material->getShader()->getProgram(Shader_Default);
		packFreePagesProgramDim = material->getLocalSize();
	}
	packFreePagesProgram->init();

	if (allocateNewPageMappingsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_AllocateNewPageMappings.mat");
		if (!material)
			throw runtime_error("allocateNewPageMappings load failed");
		allocateNewPageMappingsProgram = material->getShader()->getProgram(Shader_Default);
		allocateNewPageMappingsProgramDim = material->getLocalSize();
	}
	allocateNewPageMappingsProgram->init();

	if (generateHierarchicalPageFlagsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_GenerateHierarchicalPageFlags.mat");
		if (!material)
			throw runtime_error("generateHierarchicalPageFlags load failed");
		generateHierarchicalPageFlagsProgram = material->getShader()->getProgram(Shader_Default);
		generateHierarchicalPageFlagsProgramDim = material->getLocalSize();
	}
	generateHierarchicalPageFlagsProgram->init();

	if (clearInitPhysPagesIndirectArgsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_ClearInitPhysPagesIndirectArgs.mat");
		if (!material)
			throw runtime_error("clearInitPhysPagesIndirectArgs load failed");
		clearInitPhysPagesIndirectArgsProgram = material->getShader()->getProgram(Shader_Default);
		clearInitPhysPagesIndirectArgsProgramDim = material->getLocalSize();
	}
	clearInitPhysPagesIndirectArgsProgram->init();

	if (selectPagesToInitProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_SelectPagesToInit.mat");
		if (!material)
			throw runtime_error("selectPagesToInit load failed");
		selectPagesToInitProgram = material->getShader()->getProgram(Shader_Default);
		selectPagesToInitProgramDim = material->getLocalSize();
	}
	selectPagesToInitProgram->init();

	if (initPhysicalMemoryIndirectProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_InitPhysicalMemoryIndirect.mat");
		if (!material)
			throw runtime_error("initPhysicalMemoryIndirect load failed");
		initPhysicalMemoryIndirectProgram = material->getShader()->getProgram(Shader_Default);
		initPhysicalMemoryIndirectProgramDim = material->getLocalSize();
	}
	initPhysicalMemoryIndirectProgram->init();

	if (cullPerPageDrawCommandsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_CullPerPageDrawCommands.mat");
		if (!material)
			throw runtime_error("cullPerPageDrawCommands load failed");
		cullPerPageDrawCommandsProgram = material->getShader()->getProgram(Shader_Default);
		cullPerPageDrawCommandsProgramDim = material->getLocalSize();
	}
	cullPerPageDrawCommandsProgram->init();

	if (allocateCommandInstanceOutputSpaceProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_AllocateCommandInstanceOutputSpace.mat");
		if (!material)
			throw runtime_error("allocateCommandInstanceOutputSpace load failed");
		allocateCommandInstanceOutputSpaceProgram = material->getShader()->getProgram(Shader_Default);
		allocateCommandInstanceOutputSpaceProgramDim = material->getLocalSize();
	}
	allocateCommandInstanceOutputSpaceProgram->init();

	if (initOutputommandInstanceListsArgsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_InitOutputommandInstanceListsArgs.mat");
		if (!material)
			throw runtime_error("initOutputommandInstanceListsArgs load failed");
		initOutputommandInstanceListsArgsProgram = material->getShader()->getProgram(Shader_Default);
		initOutputommandInstanceListsArgsProgramDim = material->getLocalSize();
	}
	initOutputommandInstanceListsArgsProgram->init();

	if (outputCommandInstanceListsProgram == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Pipeline/VSM/VSM_OutputCommandInstanceLists.mat");
		if (!material)
			throw runtime_error("outputCommandInstanceLists load failed");
		outputCommandInstanceListsProgram = material->getShader()->getProgram(Shader_Default);
		outputCommandInstanceListsProgramDim = material->getLocalSize();
	}
	outputCommandInstanceListsProgram->init();
}

bool VirtualShadowMapManager::ShadowMap::isValid() const
{
	return preVirtualShadowMapID != VSM_None_ID && preRendered;
}

void VirtualShadowMapManager::ShadowMap::updateClipMap(
	int vsmID,
	const Matrix4f& worldToLightViewMatrix,
	const Vector2i& pageOffset,
	const Vector2i& cornerOffset,
	float levelRadius,
	float viewCenterZ,
	float ViewRadiusZ,
	const LightEntry& lightEntry)
{
	bool curValid = curVirtualShadowMapID != VSM_None_ID;

	if (curValid && worldToLightViewMatrix != clipmap.worldToLightViewMatrix)
		curValid = false;

	if (curValid)
	{
		float deltaZ = abs(viewCenterZ - clipmap.viewCenterZ);
		if ((deltaZ + levelRadius) > 0.9 * clipmap.viewRadiusZ)
		{
			curValid = false;
		}
	}

	if (lightEntry.preRenderFrame < 0)
		curValid = false;

	bool radiusMatched = ViewRadiusZ == clipmap.viewRadiusZ;

	if (curValid && radiusMatched)
		preVirtualShadowMapID = curVirtualShadowMapID;
	else {
		preVirtualShadowMapID = VSM_None_ID;
		clipmap.worldToLightViewMatrix = worldToLightViewMatrix;
		clipmap.viewCenterZ = viewCenterZ;
		clipmap.viewRadiusZ = ViewRadiusZ;
	}

	prePageOffset = curPageOffset;

	curVirtualShadowMapID = vsmID;
	curPageOffset = pageOffset;

	clipmap.preCornerOffset = clipmap.curCornerOffset;
	clipmap.curCornerOffset = cornerOffset;
}

VirtualShadowMapManager::ShadowMap* VirtualShadowMapManager::LightEntry::setShadowMap(int index)
{
	shadowMaps.resize(std::max(index + 1, (int)shadowMaps.size()), NULL);
	ShadowMap*& shadowMap = shadowMaps[index];
	if (index == NULL)
		shadowMap = new ShadowMap();
	return shadowMap;
}

void VirtualShadowMapManager::LightEntry::updateClipmap()
{
	preRenderFrame = std::max(preRenderFrame, curRenderFrame);
	curRenderFrame = -1;
	drawInstanceInfos.clear();
	indirectCommands.clear();
}

void VirtualShadowMapManager::LightEntry::addMeshCommand(const DirectShadowRenderCommand& command)
{
	drawInstanceInfos.reserve(drawInstanceInfos.size() + command.instanceIDCount);
	for (int i = 0; i < command.instanceIDCount; i++) {
		VSMDrawInstanceInfo& info = drawInstanceInfos.emplace_back();
		info.instanceID = command.instanceID + i;
	}
	DrawElementsIndirectCommand& indirectCmd = indirectCommands.emplace_back();
	indirectCmd.count = command.mesh->elementCount;
	indirectCmd.instanceCount = 0;
	indirectCmd.firstIndex = command.mesh->elementFirst;
	indirectCmd.baseVertex = command.mesh->vertexFirst;
	indirectCmd.baseInstance = 0;
}

void VirtualShadowMapManager::LightEntry::markRendered(unsigned int frame)
{
	curRenderFrame = frame;
}

void VirtualShadowMapManager::LightEntry::invalidate()
{
	for (auto& shadowMap : shadowMaps) {
		delete shadowMap;
	}
	shadowMaps.clear();
	drawInstanceInfos.clear();
}

VirtualShadowMapManager::VirtualShadowMapManager()
	: poolTexture(Texture2D(0, 0, 1, false,
		{ TW_Border, TW_Border, TF_Point, TF_Point, TIT_D32_F, { 255, 255, 255, 255 } }))
	, invalidationInfoBuffer(GB_Constant, GBF_Struct, sizeof(InvalidationInfo), GAF_Read, CAF_Write)
{
}

void VirtualShadowMapManager::invalidate()
{
	for (auto& lightEntry : preShadowMaps) {
		lightEntry.second->invalidate();
		delete lightEntry.second;
	}
	preShadowMaps.clear();
	for (auto& lightEntry : curShadowMaps) {
		lightEntry.second->invalidate();
		delete lightEntry.second;
	}
	curShadowMaps.clear();
}

void VirtualShadowMapManager::setPoolTextureSize(Vector2u size)
{
	if (poolTexture.getWidth() == size.x() && poolTexture.getHeight() == size.y())
		return;
	poolTexture.resize(size.x(), size.y());
	invalidate();
}

VirtualShadowMapManager::LightEntry* VirtualShadowMapManager::setLightEntry(int lightID, int cameraID)
{
	unsigned long long cacheID = (unsigned long long(lightID) << 32) | unsigned long long(cameraID);
	auto iter = curShadowMaps.find(cacheID);
	if (iter == curShadowMaps.end())
		return iter->second;

	iter = preShadowMaps.find(cacheID);
	LightEntry* shadowMap;
	if (iter == preShadowMaps.end()) {
		shadowMap = new LightEntry();
	}
	else {
		shadowMap = iter->second;
	}

	curShadowMaps.insert(make_pair(cacheID, shadowMap));

	return shadowMap;
}

void VirtualShadowMapManager::prepare()
{
	VirtualShadowMapShaders::loadDefaultResource();

}

void VirtualShadowMapManager::processInvalidations(IRenderContext& context, MeshTransformRenderData& transformData)
{
	if (!transformData.needUpdate || transformData.transformUploadIndexBuffer.empty())
		return;

	int updateCount = transformData.meshTransformDataArray.getUpdateCount();

	InvalidationInfo invalidationInfo;
	invalidationInfo.numUpdateInstances = updateCount;

	invalidationInfoBuffer.uploadData(1, &invalidationInfo, true);

	context.bindShaderProgram(VirtualShadowMapShaders::procInvalidationsProgram);

	transformData.bind(context);
	context.bindBufferBase(prevFrameData.vsmInfo->getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(prevFrameData.pageTable->getVendorGPUBuffer(), VirtualShadowMapShaders::pageTableName);
	context.bindBufferBase(prevFrameData.pageFlags->getVendorGPUBuffer(), VirtualShadowMapShaders::pageFlagsName);
	context.bindBufferBase(prevFrameData.projData->getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
	context.bindBufferBase(invalidationInfoBuffer.getVendorGPUBuffer(), VirtualShadowMapShaders::invalidationInfoName);
	context.bindBufferBase(prevFrameData.physPageMetaData->getVendorGPUBuffer(), VirtualShadowMapShaders::outPrevPhysPageMetaDataName, { true });

	const bool updateAll = transformData.meshTransformDataArray.updateAll;

	if (!updateAll) {
		context.bindBufferBase(transformData.transformUploadIndexBuffer.getVendorGPUBuffer(), VirtualShadowMapShaders::invalidationIndicesName);
	}

	context.dispatchCompute(ceil(updateCount / (float)VirtualShadowMapShaders::procInvalidationsProgramDim.x()), 1, 1);
}

void VirtualShadowMapManager::buildPrevData(const vector<VirtualShadowMap*>& shadowMaps, vector<VirtualShadowMapPrevData>& prevData)
{
	prevData.resize(shadowMaps.size());
	for (int i = 0; i < shadowMaps.size(); i++) {
		VirtualShadowMap* virtualShadowMap = shadowMaps[i];
		ShadowMap* shadowMap = virtualShadowMap ? virtualShadowMap->shadowMap : NULL;
		VirtualShadowMapPrevData& data = prevData[i];
		if (shadowMap == NULL || !shadowMap->isValid()) {
			data.vsmID = shadowMap->preVirtualShadowMapID;

			const Vector2i& curOffset = shadowMap->clipmap.curCornerOffset;
			const Vector2i& prevOffset = shadowMap->clipmap.preCornerOffset;
			data.ClipmapCornerOffsetDelta = prevOffset - curOffset;
		}
		else {
			data.vsmID = VSM_None_ID;
		}
	}
}

void VirtualShadowMapManager::bindPrevFrameData(IRenderContext& context)
{
	context.bindBufferBase(prevFrameData.pageTable->getVendorGPUBuffer(), VirtualShadowMapShaders::prevPageTableName);
	context.bindBufferBase(prevFrameData.pageFlags->getVendorGPUBuffer(), VirtualShadowMapShaders::prevPageFlagsName);
	context.bindBufferBase(prevFrameData.physPageMetaData->getVendorGPUBuffer(), VirtualShadowMapShaders::prevPhysPageMetaDataName);
}

VirtualShadowMap::VirtualShadowMap(int vsmID) : vsmID(vsmID)
{
}

VirtualShadowMapArray::VirtualShadowMapArray()
	: manager(NULL), physPagePool(NULL)
	, genPageFlagInfoBuffer(GB_Constant, GBF_Struct, sizeof(VirtualShadowMapInfo), GAF_Read, CAF_Write)
	, vsmPrevData(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapPrevData), GAF_Read, CAF_Write)
	, vsmInfo(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapInfo), GAF_Read, CAF_Write)
	, projData(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapProjectionData), GAF_ReadWrite, CAF_None)
	, pageRect(GB_Storage, GBF_Struct, sizeof(Vector4u), GAF_ReadWrite, CAF_None)
	, pageRequestFlags(GB_Storage, GBF_Struct, sizeof(Vector4u), GAF_ReadWrite, CAF_None)
	, pageTable(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)
	, pageFlags(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)
	, freePhysPages(GB_Storage, GBF_Struct, sizeof(int), GAF_ReadWrite, CAF_None)
	, physPageAllocRequests(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapPhysicalPageRequest), GAF_ReadWrite, CAF_None)
	, physPageMetaData(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapPhysicalPageData), GAF_ReadWrite, CAF_None)
	, allocPageRect(GB_Storage, GBF_Struct, sizeof(Vector4u), GAF_ReadWrite, CAF_None)
	, initPhysPagesIndirectArgs(GB_Command, GBF_Raw, sizeof(unsigned int), GAF_ReadWrite, CAF_None)
	, physPagesToInit(GB_Storage, GBF_Struct, sizeof(int), GAF_ReadWrite, CAF_None)
	, shadowViewInfos(GB_Storage, GBF_Struct, sizeof(ShadowViewInfo), GAF_Read, CAF_Write)
{
}

VirtualShadowMapArray::CullingBatchInfo::CullingBatchInfo()
	: cullingData(GB_Constant, GBF_Struct, sizeof(CullingData), GAF_Read, CAF_Write)
	, allocCmdInfo(GB_Constant, GBF_Struct, sizeof(AllocCmdInfo), GAF_Read, CAF_Write)

	, visiableInstanceInfos(GB_Storage, GBF_Struct, sizeof(VisiableInstanceInfo), GAF_ReadWrite, CAF_None)
	, visiableInstanceCount(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)

	, offsetBufferCount(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)

	, outputCommandListsIndirectArgs(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, shadowDepthIndirectArgs(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None)

	, shadowDepthInstanceOffset(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, shadowDepthInstanceCounter(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)

	, drawInstanceInfos(GB_Storage, GBF_Struct, sizeof(VSMDrawInstanceInfo), GAF_ReadWrite, CAF_Write)
	, pageInfo(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
{

}

void VirtualShadowMapArray::init(IRenderContext& context, VirtualShadowMapManager* manager)
{
	this->manager = manager;
	if (manager == NULL)
		return;

	VirtualShadowMapConfig& config = VirtualShadowMapConfig::instance();

	Vector2u physPagesXY = config.getPhysPagesXY();
	Vector2u physPoolSize = physPagesXY * config.pageSize;

	vsmInfoUpload.maxPhysPages = physPagesXY.x() * physPagesXY.y();
	vsmInfoUpload.physPageRowMask = physPagesXY.x() - 1;
	vsmInfoUpload.physPageRowShift = log2((float)physPagesXY.x());
	vsmInfoUpload.physPoolSize = physPoolSize;
	vsmInfoUpload.physPoolPages = physPagesXY;

	manager->setPoolTextureSize(physPoolSize);
	physPagePool = &manager->poolTexture;
}

VirtualShadowMap* VirtualShadowMapArray::allocate()
{
	VirtualShadowMap* vsm = new VirtualShadowMap(shadowMaps.size());
	shadowMaps.push_back(vsm);
	return vsm;
}

VirtualShadowMapArray::~VirtualShadowMapArray()
{
	for (int i = 0; i < directLightVSMIDs.size(); i++)
		delete directLightVSMIDs[i];
	directLightVSMIDs.clear();
	for (int i = 0; i < cachedCullingBatchInfos.size(); i++)
		delete cachedCullingBatchInfos[i];
	cachedCullingBatchInfos.clear();
}

void VirtualShadowMapArray::buildPageAllocations(
	IRenderContext& context,
	Texture2D& sceneDepthMap,
	Texture2D& sceneNormalMap,
	const vector<CameraRenderData*>& cameraDatas,
	const LightRenderData& lightData)
{
	if (shadowMaps.empty() || cameraDatas.empty())
		return;

	VirtualShadowMapConfig& config = VirtualShadowMapConfig::instance();

	const int shadowMapCount = shadowMaps.size();

	vector<VirtualShadowMapProjectionData> projDataUpload(shadowMapCount);
	for (VirtualShadowMapClipmap* clipmap : lightData.mainLightClipmaps) {
		for (int clipmapIndex = 0; clipmapIndex < clipmap->getLevelCount(); clipmapIndex++) {
			unsigned int vsmID = clipmap->getVirtualShadowMap(clipmapIndex)->vsmID;
			clipmap->getProjectData(clipmapIndex, projDataUpload[vsmID]);
		}
	}

	vector<VirtualShadowMapPrevData> vsmPrevDataUpload;
	manager->buildPrevData(shadowMaps, vsmPrevDataUpload);
	vsmPrevData.uploadData(vsmPrevDataUpload.size(), vsmPrevDataUpload.data());

	projData.uploadData(shadowMapCount, shadowMaps.data());

	vsmInfoUpload.numShadowMapSlots = shadowMapCount;
	vsmInfo.uploadData(1, &vsmInfoUpload);

	const int pageFlagsCount = std::max(128 * 1024, shadowMapCount * int(VirtualShadowMapConfig::pageTableSize));
	pageRequestFlags.resize(pageFlagsCount);
	context.clearOutputBufferUint(pageRequestFlags.getVendorGPUBuffer(), Vector4u::Zero());

	// not use hzb
	//dirtyPageFlags.resize(vsmInfoUpload.maxPhysPages);
	//context.clearOutputBufferUint(dirtyPageFlags.getVendorGPUBuffer(), Vector4u::Zero());

	const int pageRectCount = shadowMaps.size() * VirtualShadowMapConfig::maxMips;
	pageRect.resize(pageRectCount);
	context.bindShaderProgram(VirtualShadowMapShaders::initPageRectsProgram);
	context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(pageRect.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageRequestFlagsName, { true });
	context.dispatchCompute(ceil(pageRectCount / (float)VirtualShadowMapShaders::initPageRectsProgramDim.x()), 1, 1);

	GenPageFlagInfo genPageFlagInfo;
	genPageFlagInfo.directLightCount = true;
	genPageFlagInfo.pageDilationBorderSizeMain = config.pageDilationBorderSizeMain;
	genPageFlagInfo.pageDilationBorderSizeLocal = config.pageDilationBorderSizeLocal;
	genPageFlagInfo.backFaceCull = true;

	genPageFlagInfoBuffer.uploadData(1, &genPageFlagInfo, true);

	resizeDirectLightVSMIDs(cameraDatas);

	int camIndex = 0;
	for (auto& cameraData : cameraDatas) {
		context.bindShaderProgram(VirtualShadowMapShaders::generatePageFlagsFromPixelsProgram);

		vector<unsigned int> directLightVSMIDsUpload;
		for (VirtualShadowMapClipmap* clipmap : lightData.mainLightClipmaps) {
			if (clipmap->getCameraRenderData() == cameraData) {
				directLightVSMIDsUpload.push_back(clipmap->getVirtualShadowMap(0)->vsmID);
			}
		}

		if (directLightVSMIDsUpload.empty())
			continue;

		GPUBuffer* curDirectLightVSMIDs = directLightVSMIDs[camIndex];

		curDirectLightVSMIDs->uploadData(directLightVSMIDsUpload.size(), directLightVSMIDsUpload.data(), true);

		cameraData->bind(context);
		context.bindBufferBase(genPageFlagInfoBuffer.getVendorGPUBuffer(), VirtualShadowMapShaders::GenPageFlagInfoName);
		context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
		context.bindBufferBase(curDirectLightVSMIDs->getVendorGPUBuffer(), VirtualShadowMapShaders::directLightVSMIDsName);
		context.bindBufferBase(pageRequestFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageRequestFlagsName, { true });
		context.bindTexture(sceneDepthMap.getVendorTexture(), VirtualShadowMapShaders::sceneDepthMapName);
		context.bindTexture(sceneNormalMap.getVendorTexture(), VirtualShadowMapShaders::sceneNormalMapName);
		context.dispatchCompute(
			ceil(cameraData->data.viewSize.x() / (float)VirtualShadowMapShaders::generatePageFlagsFromPixelsProgramDim.x()),
			ceil(cameraData->data.viewSize.y() / (float)VirtualShadowMapShaders::generatePageFlagsFromPixelsProgramDim.y()),
			1);
		camIndex++;
	}

	pageTable.resize(pageFlagsCount);
	pageFlags.resize(pageFlagsCount);

	// One additional element as the last element is used as an atomic counter
	freePhysPages.resize(vsmInfoUpload.maxPhysPages + 1);
	physPageAllocRequests.resize(vsmInfoUpload.maxPhysPages + 1);

	physPageMetaData.resize(vsmInfoUpload.maxPhysPages);
	allocPageRect.resize(pageRectCount);

	context.bindShaderProgram(VirtualShadowMapShaders::initPhysicalPageMetaDataProgram);
	context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageMetaDataName, { true });
	context.bindBufferBase(freePhysPages.getVendorGPUBuffer(), VirtualShadowMapShaders::outFreePhysPagesName, { true });
	context.bindBufferBase(physPageAllocRequests.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageAllocRequestsName, { true });
	context.dispatchCompute(ceil(vsmInfoUpload.maxPhysPages / (float)VirtualShadowMapShaders::initPhysicalPageMetaDataProgramDim.x()), 1, 1);

	context.bindShaderProgram(VirtualShadowMapShaders::createCachedPageMappingsProgram);
	context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
	context.bindBufferBase(vsmPrevData.getVendorGPUBuffer(), VirtualShadowMapShaders::vsmPrevDataName);
	manager->bindPrevFrameData(context);
	context.bindBufferBase(pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageTableName, { true });
	context.bindBufferBase(pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageFlagsName, { true });
	context.bindBufferBase(physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageMetaDataName, { true });
	context.bindBufferBase(physPageAllocRequests.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageAllocRequestsName, { true });
	context.dispatchCompute(ceil(config.pageTableSize / (float)VirtualShadowMapShaders::createCachedPageMappingsProgramDim.x()), 1, 1);

	context.bindShaderProgram(VirtualShadowMapShaders::packFreePagesProgram);
	context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageMetaDataName);
	context.bindBufferBase(freePhysPages.getVendorGPUBuffer(), VirtualShadowMapShaders::outFreePhysPagesName, { true });
	context.dispatchCompute(ceil(vsmInfoUpload.maxPhysPages / (float)VirtualShadowMapShaders::packFreePagesProgramDim.x()), 1, 1);

	context.bindShaderProgram(VirtualShadowMapShaders::allocateNewPageMappingsProgram);
	context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(pageRequestFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::pageRequestFlagsName);
	context.bindBufferBase(physPageAllocRequests.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageAllocRequestsName);
	context.bindBufferBase(pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageTableName, { true });
	context.bindBufferBase(pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageFlagsName, { true });
	context.bindBufferBase(freePhysPages.getVendorGPUBuffer(), VirtualShadowMapShaders::outFreePhysPagesName, { true });
	context.bindBufferBase(physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageMetaDataName, { true });
	context.dispatchCompute(ceil(vsmInfoUpload.maxPhysPages / (float)VirtualShadowMapShaders::allocateNewPageMappingsProgramDim.x()), 1, 1);

	context.bindShaderProgram(VirtualShadowMapShaders::generateHierarchicalPageFlagsProgram);
	context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
	context.bindBufferBase(pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageFlagsName, { true });
	context.bindBufferBase(pageRect.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageRectName, { true });
	context.dispatchCompute(ceil(vsmInfoUpload.maxPhysPages / (float)VirtualShadowMapShaders::generateHierarchicalPageFlagsProgramDim.x()), 1, 1);

	{
		// Skip PropagateMappedMips, for only having clipmaps
	}

	context.bindShaderProgram(VirtualShadowMapShaders::clearInitPhysPagesIndirectArgsProgram);
	context.bindBufferBase(initPhysPagesIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outInitPhysPagesIndirectArgsName, { true });
	context.dispatchCompute(1, 1, 1);

	context.bindShaderProgram(VirtualShadowMapShaders::selectPagesToInitProgram);
	context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageMetaDataName);
	context.bindBufferBase(initPhysPagesIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outInitPhysPagesIndirectArgsName, { true });
	context.bindBufferBase(physPagesToInit.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPagesToInitName, { true });
	context.dispatchCompute(ceil(vsmInfoUpload.maxPhysPages / (float)VirtualShadowMapShaders::selectPagesToInitProgramDim.x()), 1, 1);

	context.bindShaderProgram(VirtualShadowMapShaders::initPhysicalMemoryIndirectProgram);
	context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
	context.bindBufferBase(physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageMetaDataName);
	context.bindBufferBase(physPagesToInit.getVendorGPUBuffer(), VirtualShadowMapShaders::physPagesToInitName);
	Image image;
	image.texture = physPagePool;
	context.bindImage(image, VirtualShadowMapShaders::outPhysPagePoolName);
	context.dispatchComputeIndirect(initPhysPagesIndirectArgs.getVendorGPUBuffer(), 0);

	context.clearOutputBufferBindings();
}

void VirtualShadowMapArray::clean()
{
	cullingBatchInfos.clear();
	shadowViewInfosUpload.clear();
}

void VirtualShadowMapArray::resizeDirectLightVSMIDs(const vector<CameraRenderData*>& cameraDatas)
{
	int diff = cameraDatas.size() - directLightVSMIDs.size();
	if (diff > 0) {
		for (int i = 0; i < diff; i++)
			directLightVSMIDs.push_back(new GPUBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None));
	}
	else if (diff < 0) {
		for (int i = directLightVSMIDs.size() - 1; i >= cameraDatas.size(); i--)
			delete directLightVSMIDs[i];
		directLightVSMIDs.resize(cameraDatas.size(), NULL);
	}
}

VirtualShadowMapArray::CullingBatchInfo* VirtualShadowMapArray::fetchCullingBatchInfo()
{
	int index = cullingBatchInfos.size();
	CullingBatchInfo* info = NULL;
	if (index < cachedCullingBatchInfos.size()) {
		info = cachedCullingBatchInfos[index];
	}
	else {
		info = cachedCullingBatchInfos.emplace_back(new CullingBatchInfo);
	}
	info->data.firstView = 0;
	info->data.viewCount = 0;
	info->data.maxPerInstanceCmdCount = VirtualShadowMapConfig::maxPerInstanceCmdCount;
	info->data.frame = Time::frames();
	cullingBatchInfos.push_back(info);
	return info;
}

void VirtualShadowMapArray::buildCullingBatchInfos(const LightRenderData& lightData)
{
	for (VirtualShadowMapClipmap* clipmap : lightData.mainLightClipmaps) {
		CullingBatchInfo* cullingBatchInfo = fetchCullingBatchInfo();
		cullingBatchInfo->data.firstView = shadowViewInfosUpload.size();
		cullingBatchInfo->lightEntry = clipmap->getLightEntry();
		if (cullingBatchInfo->lightEntry) {
			int indirectDrawCount = cullingBatchInfo->lightEntry->drawInstanceInfos.size();
			int maxInstanceDrawPerPass = indirectDrawCount * VirtualShadowMapConfig::maxPerInstanceCmdCount;
			cullingBatchInfo->lightEntry->markRendered(Time::frames());
			cullingBatchInfo->data.instanceCount = indirectDrawCount;
			cullingBatchInfo->drawInstanceInfos.uploadData(cullingBatchInfo->data.instanceCount,
				cullingBatchInfo->lightEntry->drawInstanceInfos.data());
			cullingBatchInfo->shadowDepthIndirectArgs.uploadData(
				cullingBatchInfo->lightEntry->indirectCommands.size() * (sizeof(DrawElementsIndirectCommand) / sizeof(unsigned int)),
				cullingBatchInfo->lightEntry->indirectCommands.data());
			AllocCmdInfo allocCmdInfoUpload;
			allocCmdInfoUpload.indirectArgCount = indirectDrawCount;
			cullingBatchInfo->allocCmdInfo.uploadData(1, &allocCmdInfoUpload);
			cullingBatchInfo->visiableInstanceInfos.resize(maxInstanceDrawPerPass);
			cullingBatchInfo->visiableInstanceCount.resize(1);
			cullingBatchInfo->offsetBufferCount.resize(1);
			cullingBatchInfo->outputCommandListsIndirectArgs.resize(3);
			cullingBatchInfo->shadowDepthInstanceOffset.resize(indirectDrawCount);
			cullingBatchInfo->shadowDepthInstanceCounter.resize(indirectDrawCount);
			cullingBatchInfo->drawInstanceInfos.resize(indirectDrawCount);
			cullingBatchInfo->pageInfo.resize(indirectDrawCount);
		}
		for (int levelIndex = 0; levelIndex < clipmap->getLevelCount(); levelIndex++) {
			VirtualShadowMap* vsm = clipmap->getVirtualShadowMap(levelIndex);
			if (vsm == NULL)
				continue;
			ShadowViewInfo& view = shadowViewInfosUpload.emplace_back();
			view.viewRect = Vector4i(0, 0, VirtualShadowMapConfig::virtualShadowMapSize, VirtualShadowMapConfig::virtualShadowMapSize);
			view.vsmID = vsm->vsmID;
			view.flags = VSM_DirectLight;
			view.mipLevel = 0;
			view.mipCount = 1;
			cullingBatchInfo->data.viewCount++;
		}
		cullingBatchInfo->cullingData.uploadData(1, &cullingBatchInfo->data, true);
	}
}

void VirtualShadowMapArray::cullingPasses(IRenderContext& context, const LightRenderData& lightData)
{
	buildCullingBatchInfos(lightData);
	shadowViewInfos.uploadData(shadowViewInfosUpload.size(), shadowViewInfosUpload.data());
	for (CullingBatchInfo* cullingBatchInfo : cullingBatchInfos) {
		int indirectDrawCount = cullingBatchInfo->data.instanceCount;

		context.bindShaderProgram(VirtualShadowMapShaders::cullPerPageDrawCommandsProgram);
		context.clearOutputBufferUint(cullingBatchInfo->visiableInstanceCount.getVendorGPUBuffer(), Vector4u::Zero());
		context.bindBufferBase(vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMBuffInfoName);
		context.bindBufferBase(projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
		context.bindBufferBase(pageRect.getVendorGPUBuffer(), VirtualShadowMapShaders::pageRectName);
		context.bindBufferBase(shadowViewInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::shadowViewInfosName);
		context.bindBufferBase(cullingBatchInfo->cullingData.getVendorGPUBuffer(), VirtualShadowMapShaders::CullingDataName);
		context.bindBufferBase(cullingBatchInfo->drawInstanceInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::drawInstanceInfosName);
		context.bindBufferBase(cullingBatchInfo->visiableInstanceInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::outVisiableInstanceInfosName, { true });
		context.bindBufferBase(cullingBatchInfo->visiableInstanceCount.getVendorGPUBuffer(), VirtualShadowMapShaders::outVisiableInstanceCountName, { true });
		context.bindBufferBase(cullingBatchInfo->shadowDepthIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outShadowDepthIndirectArgsName, { true });
		context.dispatchCompute(ceil(indirectDrawCount / (float)VirtualShadowMapShaders::cullPerPageDrawCommandsProgramDim.x()), 1, 1);

		context.bindShaderProgram(VirtualShadowMapShaders::allocateCommandInstanceOutputSpaceProgram);
		context.clearOutputBufferUint(cullingBatchInfo->shadowDepthInstanceOffset.getVendorGPUBuffer(), Vector4u::Zero());
		context.bindBufferBase(cullingBatchInfo->allocCmdInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::AllocCmdInfoName);
		context.bindBufferBase(cullingBatchInfo->shadowDepthIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outShadowDepthIndirectArgsName);
		context.bindBufferBase(cullingBatchInfo->offsetBufferCount.getVendorGPUBuffer(), VirtualShadowMapShaders::outOffsetBufferCountName, { true });
		context.bindBufferBase(cullingBatchInfo->shadowDepthInstanceOffset.getVendorGPUBuffer(), VirtualShadowMapShaders::outShadowDepthInstanceOffsetName, { true });
		context.bindBufferBase(cullingBatchInfo->shadowDepthInstanceCounter.getVendorGPUBuffer(), VirtualShadowMapShaders::shadowDepthInstanceCounterName, { true });
		context.dispatchCompute(ceil(indirectDrawCount / (float)VirtualShadowMapShaders::allocateCommandInstanceOutputSpaceProgramDim.x()), 1, 1);

		context.bindShaderProgram(VirtualShadowMapShaders::initOutputommandInstanceListsArgsProgram);
		context.bindBufferBase(cullingBatchInfo->offsetBufferCount.getVendorGPUBuffer(), VirtualShadowMapShaders::offsetBufferCountName);
		context.bindBufferBase(cullingBatchInfo->outputCommandListsIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outShadowDepthInstanceOffsetName, { true });
		context.dispatchCompute(1, 1, 1);

		context.bindShaderProgram(VirtualShadowMapShaders::outputCommandInstanceListsProgram);
		context.bindBufferBase(cullingBatchInfo->visiableInstanceInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::visiableInstanceInfosName);
		context.bindBufferBase(cullingBatchInfo->visiableInstanceCount.getVendorGPUBuffer(), VirtualShadowMapShaders::visiableInstanceCountName);
		context.bindBufferBase(cullingBatchInfo->shadowDepthInstanceCounter.getVendorGPUBuffer(), VirtualShadowMapShaders::shadowDepthInstanceCounterName, { true });
		context.bindBufferBase(cullingBatchInfo->drawInstanceInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::outDrawInstanceInfosName, { true });
		context.bindBufferBase(cullingBatchInfo->pageInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageInfoName, { true });
		context.dispatchComputeIndirect(cullingBatchInfo->outputCommandListsIndirectArgs.getVendorGPUBuffer(), 0);
	}
}
