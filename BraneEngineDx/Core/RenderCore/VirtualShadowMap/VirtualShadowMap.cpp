#include "VirtualShadowMap.h"
#include "VirtualShadowMapClipmap.h"
#include "../RenderTask.h"
#include "../SurfaceBufferGetter.h"
#include "../DirectShadowRenderPack.h"
#include "../../Utility/MathUtility.h"
#include "../../Engine.h"
#include "../../Asset.h"
#include "../../InitializationManager.h"

SerializeInstance(VirtualShadowMapConfig);

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

void VirtualShadowMapConfig::setEnable(bool enable)
{
	instance().enable = enable;
}

bool VirtualShadowMapConfig::isEnable()
{
	return instance().enable;
}

Serializable* VirtualShadowMapConfig::instantiate(const SerializationInfo& from)
{
	return new VirtualShadowMapConfig();
}

bool VirtualShadowMapConfig::deserialize(const SerializationInfo& from)
{
	int enableValue = 0;
	if (from.get("enable", enableValue))
		enable = enableValue;
	from.get("physPoolWidth", (int&)physPoolWidth);
	from.get("maxPhysPages", (int&)maxPhysPages);
	from.get("firstClipmapLevel", (int&)firstClipmapLevel);
	from.get("lastClipmapLevel", (int&)lastClipmapLevel);
	from.get("clipmapRadiusZScale", clipmapRadiusZScale);
	from.get("resolutionLodBiasClipmap", resolutionLodBiasClipmap);
	from.get("pageDilationBorderSizeMain", pageDilationBorderSizeMain);
	from.get("pageDilationBorderSizeLocal", pageDilationBorderSizeLocal);
	from.get("debugViewMode", debugViewMode);
	from.get("debugVSMID", debugVSMID);
	return false;
}

bool VirtualShadowMapConfig::serialize(SerializationInfo& to)
{
	serializeInit(this, to);
	to.set("enable", (int)enable);
	to.set("physPoolWidth", (int&)physPoolWidth);
	to.set("maxPhysPages", (int&)maxPhysPages);
	to.set("firstClipmapLevel", (int&)firstClipmapLevel);
	to.set("lastClipmapLevel", (int&)lastClipmapLevel);
	to.set("clipmapRadiusZScale", clipmapRadiusZScale);
	to.set("resolutionLodBiasClipmap", resolutionLodBiasClipmap);
	to.set("pageDilationBorderSizeMain", pageDilationBorderSizeMain);
	to.set("pageDilationBorderSizeLocal", pageDilationBorderSizeLocal);
	to.set("debugViewMode", debugViewMode);
	to.set("debugVSMID", debugVSMID);
	return false;
}

class VirtualShadowMapConfigInitializer : public Initialization
{
protected:
	static VirtualShadowMapConfigInitializer instance;
	VirtualShadowMapConfigInitializer() : Initialization(
		InitializeStage::BeforeEngineSetup, 0,
		FinalizeStage::BeforeEngineRelease, 0) { }

	virtual bool initialize()
	{
		Engine::engineConfig.configInfo.get("virtualShadowMap", VirtualShadowMapConfig::instance());
		return true;
	}

	virtual bool finalize()
	{
		return true;
	}
};

VirtualShadowMapConfigInitializer VirtualShadowMapConfigInitializer::instance;

VirtualShadowMapFrameData::VirtualShadowMapFrameData()
	: vsmInfo(GB_Constant, GBF_Struct, sizeof(VirtualShadowMapInfo), GAF_Read, CAF_Write)
	, pageTable(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)
	, pageFlags(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)
	, projData(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapProjectionData), GAF_ReadWrite, CAF_None)
	, pageRect(GB_Storage, GBF_Struct, sizeof(Vector4u), GAF_ReadWrite, CAF_None)
	, physPageMetaData(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapPhysicalPageData), GAF_ReadWrite, CAF_None)
{
}

VirtualShadowMapFrameData::~VirtualShadowMapFrameData()
{
	release();
}

void VirtualShadowMapFrameData::release()
{
	vsmInfo.resize(0);
	pageTable.resize(0);
	pageFlags.resize(0);
	projData.resize(0);
	pageRect.resize(0);
	physPageMetaData.resize(0);
}

#define VSM_SHADER_IMP(name) \
ShaderProgram* VirtualShadowMapShaders::name##Program = NULL; \
ShaderProgram* VirtualShadowMapShaders::name##DebugProgram = NULL; \
Vector3u VirtualShadowMapShaders::name##ProgramDim;

VSM_SHADER_IMP(procInvalidations);
VSM_SHADER_IMP(initPageRects);
VSM_SHADER_IMP(generatePageFlagsFromPixels);
VSM_SHADER_IMP(initPhysicalPageMetaData);
VSM_SHADER_IMP(createCachedPageMappingsHasCache);
VSM_SHADER_IMP(createCachedPageMappingsNoCache);
VSM_SHADER_IMP(packFreePages);
VSM_SHADER_IMP(allocateNewPageMappings);
VSM_SHADER_IMP(generateHierarchicalPageFlags);
VSM_SHADER_IMP(clearInitPhysPagesIndirectArgs);
VSM_SHADER_IMP(selectPagesToInit);
VSM_SHADER_IMP(initPhysicalMemoryIndirect);
VSM_SHADER_IMP(cullPerPageDrawCommands);
VSM_SHADER_IMP(allocateCommandInstanceOutputSpace);
VSM_SHADER_IMP(initOutputommandInstanceListsArgs);
VSM_SHADER_IMP(outputCommandInstanceLists);
VSM_SHADER_IMP(debugBlit);
VSM_SHADER_IMP(debugClipmap);

const ShaderPropertyName VirtualShadowMapShaders::VSMInfoBuffName = "VSMInfoBuff";
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

const ShaderPropertyName VirtualShadowMapShaders::outVisiableInstanceInfosName = "outVisiableInstanceInfos";
const ShaderPropertyName VirtualShadowMapShaders::outVisiableInstanceCountName = "outVisiableInstanceCount";
const ShaderPropertyName VirtualShadowMapShaders::outOffsetBufferCountName = "outOffsetBufferCount";
const ShaderPropertyName VirtualShadowMapShaders::outShadowDepthInstanceOffsetName = "outShadowDepthInstanceOffset";

const ShaderPropertyName VirtualShadowMapShaders::outputCommandListsIndirectArgsName = "outputCommandListsIndirectArgs";
const ShaderPropertyName VirtualShadowMapShaders::outShadowDepthIndirectArgsName = "outShadowDepthIndirectArgs";

const ShaderPropertyName VirtualShadowMapShaders::instanceIDsName = "instanceIDs";
const ShaderPropertyName VirtualShadowMapShaders::outInstanceIDsName = "outInstanceIDs";

const ShaderPropertyName VirtualShadowMapShaders::pageInfoName = "pageInfo";
const ShaderPropertyName VirtualShadowMapShaders::outPageInfoName = "outPageInfo";

const ShaderPropertyName VirtualShadowMapShaders::debugBufferName = "debugBuffer";
const ShaderPropertyName VirtualShadowMapShaders::outDebugBufferName = "outDebugBuffer";

#define VSM_SHADER_PATH_PREFIX "Engine/Shaders/Pipeline/VSM/"
#define VSM_LOAD_SHADER(name, filename, feature)										\
if (name##Program == NULL) {															\
	Material* material = getAssetByPath<Material>(VSM_SHADER_PATH_PREFIX##filename);	\
	if (!material)																		\
		throw runtime_error(#name" load failed");										\
	name##Program = material->getShader()->getProgram(feature);							\
	name##ProgramDim = material->getLocalSize();										\
}																						\
name##Program->init();

#define VSM_LOAD_SHADER_WITH_DEBUG(name, filename, feature)								\
if (name##Program == NULL) {															\
	Material* material = getAssetByPath<Material>(VSM_SHADER_PATH_PREFIX##filename);	\
	if (!material)																		\
		throw runtime_error(#name" load failed");										\
	name##Program = material->getShader()->getProgram(feature);							\
	name##DebugProgram = material->getShader()->getProgram(feature | Shader_Debug);		\
	name##ProgramDim = material->getLocalSize();										\
}																						\
name##Program->init();																	\
if (name##DebugProgram)																	\
	name##DebugProgram->init();

#define VSM_LOAD_SHADER_DEFAULT(name, filename) VSM_LOAD_SHADER(name, filename, Shader_Default)
#define VSM_LOAD_SHADER_DEFAULT_WITH_DEBUG(name, filename) VSM_LOAD_SHADER_WITH_DEBUG(name, filename, Shader_Default)

void VirtualShadowMapShaders::loadDefaultResource()
{
	VSM_LOAD_SHADER_DEFAULT(procInvalidations, "VSM_ProcInvalidations.mat");
	VSM_LOAD_SHADER_DEFAULT(initPageRects, "VSM_InitPageRects.mat");
	VSM_LOAD_SHADER_DEFAULT_WITH_DEBUG(generatePageFlagsFromPixels, "VSM_GeneratePageFlagsFromPixels.mat");
	VSM_LOAD_SHADER_DEFAULT(initPhysicalPageMetaData, "VSM_InitPhysicalPageMetaData.mat");

	enum
	{
		NoCache = Shader_Default,
		HasCache = Shader_Custom_1,
	};

	VSM_LOAD_SHADER(createCachedPageMappingsNoCache, "VSM_CreateCachedPageMappings.mat", NoCache);
	VSM_LOAD_SHADER(createCachedPageMappingsHasCache, "VSM_CreateCachedPageMappings.mat", HasCache);

	VSM_LOAD_SHADER_DEFAULT(packFreePages, "VSM_PackFreePages.mat");
	VSM_LOAD_SHADER_DEFAULT(allocateNewPageMappings, "VSM_AllocateNewPageMappings.mat");
	VSM_LOAD_SHADER_DEFAULT(generateHierarchicalPageFlags, "VSM_GenerateHierarchicalPageFlags.mat");
	VSM_LOAD_SHADER_DEFAULT(clearInitPhysPagesIndirectArgs, "VSM_ClearInitPhysPagesIndirectArgs.mat");
	VSM_LOAD_SHADER_DEFAULT(selectPagesToInit, "VSM_SelectPagesToInit.mat");
	VSM_LOAD_SHADER_DEFAULT(initPhysicalMemoryIndirect, "VSM_InitPhysicalMemoryIndirect.mat");
	VSM_LOAD_SHADER_DEFAULT(cullPerPageDrawCommands, "VSM_CullPerPageDrawCommands.mat");
	VSM_LOAD_SHADER_DEFAULT(allocateCommandInstanceOutputSpace, "VSM_AllocateCommandInstanceOutputSpace.mat");
	VSM_LOAD_SHADER_DEFAULT(initOutputommandInstanceListsArgs, "VSM_InitOutputCommandInstanceListsArgs.mat");
	VSM_LOAD_SHADER_DEFAULT(outputCommandInstanceLists, "VSM_OutputCommandInstanceLists.mat");
	VSM_LOAD_SHADER(debugBlit, "VSM_DebugBlit.mat", Shader_Debug);
	VSM_LOAD_SHADER(debugClipmap, "VSM_DebugClipmap.mat", Shader_Debug);
}

bool VSMInstanceDrawResource::ExecutionOrder::operator()(const VSMInstanceDrawResource& t0, const VSMInstanceDrawResource& t1) const
{
	if (t0.shaderProgram < t1.shaderProgram)
		return true;
	if (t0.shaderProgram == t1.shaderProgram) {
		if (t0.transformData < t1.transformData)
			return true;
		if (t0.transformData == t1.transformData) {
			if (t0.meshData < t1.meshData)
				return true;
			if (t0.meshData == t1.meshData)
				return t0.materialData < t1.materialData;
		}
	}
	return false;
}

bool VSMInstanceDrawResource::ExecutionOrder::operator()(const VSMInstanceDrawResource* t0, const VSMInstanceDrawResource* t1) const
{
	return (*this)(*t0, *t1);
}

bool VirtualShadowMapManager::ShadowMap::isValid() const
{
	return preVirtualShadowMapID != VSM_None_ID;
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
	if (shadowMap == NULL)
		shadowMap = new ShadowMap();
	return shadowMap;
}

void VirtualShadowMapManager::LightEntry::updateClipmap()
{
	preRenderFrame = std::max(preRenderFrame, curRenderFrame);
	curRenderFrame = -1;
	batches.clear();
}

void VirtualShadowMapManager::LightEntry::addMeshCommand(const VSMMeshTransformIndexArray::CallItem& callItem)
{
	IRenderData* transformData = callItem.second.payload.transformData;
	MeshPart* mesh = callItem.first.meshPart;
	if (transformData == NULL || mesh == NULL || callItem.second.batchCount == 0)
		return;
	Batch keyBatch = { transformData };
	auto iter = batches.find(keyBatch);
	Batch* batch;
	if (iter == batches.end())
		batch = (Batch*)&*batches.insert(keyBatch).first;
	else
		batch = (Batch*)&*iter;
	const VSMMeshTransformIndex& index = callItem.second;
	const unsigned int commandIndex = batch->indirectCommands.size();
	batch->drawInstanceInfos.reserve(batch->drawInstanceInfos.size() + index.batchCount);
	for (int i = 0; i < index.batchCount; i++) {
		VSMDrawInstanceInfo& info = batch->drawInstanceInfos.emplace_back();
		info.instanceID = index.indices[i].instanceID;
		info.indirectArgIndex = commandIndex;
	}

	DrawElementsIndirectCommand& indirectCmd = batch->indirectCommands.emplace_back();
	indirectCmd.count = mesh->elementCount;
	indirectCmd.instanceCount = 0;
	indirectCmd.firstIndex = mesh->elementFirst;
	indirectCmd.baseVertex = mesh->vertexFirst;
	indirectCmd.baseInstance = 0;

	VSMInstanceDrawResource& resource = batch->resources.emplace_back();
	resource.materialData = dynamic_cast<MaterialRenderData*>(callItem.first.material->getRenderData());
	resource.meshData = callItem.first.meshPart->meshData;
	resource.transformData = callItem.second.payload.transformData;
	resource.shaderProgram = callItem.second.payload.shaderProgram;
	resource.extraData = callItem.second.payload.bindings;
}

void VirtualShadowMapManager::LightEntry::clean()
{
	//batches.clear();
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
	batches.clear();
}

VirtualShadowMapManager::VirtualShadowMapManager()
	: poolTexture(Texture2D(0, 0, 1, false,
		{ TW_Border, TW_Border, TF_Point, TF_Point, TIT_R32_UI, { 255, 255, 255, 255 } }))
	, invalidationInfoBuffer(GB_Constant, GBF_Struct, sizeof(InvalidationInfo), GAF_Read, CAF_Write)
	, curFrameData(&frameData[0])
	, prevFrameData(NULL)
{
	poolTexture.setAutoGenMip(false);
}

bool VirtualShadowMapManager::isCacheValid() const
{
	return prevFrameData;
}

VirtualShadowMapFrameData& VirtualShadowMapManager::getCurFrameData()
{
	return *curFrameData;
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
	if (iter != curShadowMaps.end())
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

void VirtualShadowMapManager::buildPrevData(const vector<VirtualShadowMap*>& shadowMaps, vector<VirtualShadowMapPrevData>& prevData)
{
	prevData.resize(shadowMaps.size());
	for (int i = 0; i < shadowMaps.size(); i++) {
		VirtualShadowMap* virtualShadowMap = shadowMaps[i];
		ShadowMap* shadowMap = virtualShadowMap ? virtualShadowMap->shadowMap : NULL;
		VirtualShadowMapPrevData& data = prevData[i];
		if (shadowMap == NULL || !shadowMap->isValid()) {
			data.vsmID = VSM_None_ID;
		}
		else {
			data.vsmID = shadowMap->preVirtualShadowMapID;

			const Vector2i& curOffset = shadowMap->clipmap.curCornerOffset;
			const Vector2i& prevOffset = shadowMap->clipmap.preCornerOffset;
			data.ClipmapCornerOffsetDelta = prevOffset - curOffset;
		}
	}
}

void VirtualShadowMapManager::bindPrevFrameData(IRenderContext& context)
{
	if (prevFrameData == NULL)
		return;
	context.bindBufferBase(prevFrameData->pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::prevPageTableName);
	context.bindBufferBase(prevFrameData->pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::prevPageFlagsName);
	context.bindBufferBase(prevFrameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::prevPhysPageMetaDataName);
}

void VirtualShadowMapManager::processInvalidations(IRenderContext& context, MeshTransformRenderData& transformData)
{
	if (prevFrameData == NULL || (!transformData.needUpdate && transformData.transformUploadIndexBuffer.empty()))
		return;

	int updateCount = transformData.meshTransformDataArray.getUpdateCount();

	InvalidationInfo invalidationInfo;
	invalidationInfo.numUpdateInstances = updateCount;

	invalidationInfoBuffer.uploadData(1, &invalidationInfo, true);

	context.bindShaderProgram(VirtualShadowMapShaders::procInvalidationsProgram);

	transformData.bind(context);
	context.bindBufferBase(prevFrameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(prevFrameData->pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::pageTableName);
	context.bindBufferBase(prevFrameData->pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::pageFlagsName);
	context.bindBufferBase(prevFrameData->projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
	context.bindBufferBase(prevFrameData->pageRect.getVendorGPUBuffer(), VirtualShadowMapShaders::pageRectName);
	context.bindBufferBase(invalidationInfoBuffer.getVendorGPUBuffer(), VirtualShadowMapShaders::invalidationInfoName);
	context.bindBufferBase(prevFrameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::outPrevPhysPageMetaDataName, { true });

	const bool updateAll = transformData.meshTransformDataArray.updateAll;

	if (!updateAll) {
		context.bindBufferBase(transformData.transformUploadIndexBuffer.getVendorGPUBuffer(), VirtualShadowMapShaders::invalidationIndicesName);
	}

	context.dispatchCompute(ceil(updateCount / (float)VirtualShadowMapShaders::procInvalidationsProgramDim.x()), 1, 1);
}

void VirtualShadowMapManager::swapFrameData(VirtualShadowMapArray& virtualShadowMapArray)
{
	bool newVSVData = virtualShadowMapArray.isAllocated();

	if (newVSVData) {
		if (curFrameData == &frameData[0]) {
			curFrameData = &frameData[1];
			prevFrameData = &frameData[0];
		}
		else {
			curFrameData = &frameData[0];
			prevFrameData = &frameData[1];
		}
		prevShadowMapInfo = virtualShadowMapArray.curFrameVSMInfo;

		preShadowMaps = curShadowMaps;
		curShadowMaps.clear();
	}
	else {
		prevFrameData = NULL;
		prevShadowMapInfo.numShadowMapSlots = 0;
	}
}

VirtualShadowMap::VirtualShadowMap(int vsmID) : vsmID(vsmID)
{
}

VirtualShadowMapArray::VirtualShadowMapArray()
	: manager(NULL), physPagePool(NULL)
	, genPageFlagInfoBuffer(GB_Constant, GBF_Struct, sizeof(VirtualShadowMapInfo), GAF_Read, CAF_Write)
	, vsmPrevData(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapPrevData), GAF_Read, CAF_Write)
	, pageRequestFlags(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)
	, freePhysPages(GB_Storage, GBF_Struct, sizeof(int), GAF_ReadWrite, CAF_None)
	, physPageAllocRequests(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapPhysicalPageRequest), GAF_ReadWrite, CAF_None)
	, allocPageRect(GB_Storage, GBF_Struct, sizeof(Vector4u), GAF_ReadWrite, CAF_None)
	, initPhysPagesIndirectArgs(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, physPagesToInit(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, shadowViewInfos(GB_Storage, GBF_Struct, sizeof(ShadowViewInfo), GAF_Read, CAF_Write)
	, clipmapDebugBuffer(Texture2D(VirtualShadowMapConfig::virtualShadowMapSize, VirtualShadowMapConfig::virtualShadowMapSize,
		1, false, { TW_Border, TW_Border, TF_Point, TF_Point, TIT_RG32_F, { 1.0f, 1.0f, 1.0f, 1.0f } }))
{
	clipmapDebugBuffer.setAutoGenMip(false);
}

VirtualShadowMapArray::CullingBatchInfo::CullingBatchInfo()
	: cullingData(GB_Constant, GBF_Struct, sizeof(CullingData), GAF_Read, CAF_Write)
	, allocCmdInfo(GB_Constant, GBF_Struct, sizeof(AllocCmdInfo), GAF_Read, CAF_Write)

	, visiableInstanceInfos(GB_Storage, GBF_Struct, sizeof(VisiableInstanceInfo), GAF_ReadWrite, CAF_None)
	, visiableInstanceCount(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)

	, offsetBufferCount(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)

	, outputCommandListsIndirectArgs(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, shadowDepthIndirectArgs(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None)

	, shadowDepthInstanceOffset(GB_Vertex, GBF_UInt2, 0, GAF_ReadWrite, CAF_None)
	, shadowDepthInstanceCounter(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)

	, drawInstanceInfos(GB_Storage, GBF_Struct, sizeof(VSMDrawInstanceInfo), GAF_Read, CAF_Write)
	, instanceIDs(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)
	, pageInfo(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None)
{

}

void VirtualShadowMapArray::init(VirtualShadowMapManager& manager)
{
	this->manager = &manager;

	VirtualShadowMapConfig& config = VirtualShadowMapConfig::instance();

	Vector2u physPagesXY = config.getPhysPagesXY();
	Vector2u physPoolSize = physPagesXY * config.pageSize;

	curFrameVSMInfo.maxPhysPages = physPagesXY.x() * physPagesXY.y();
	curFrameVSMInfo.physPageRowMask = physPagesXY.x() - 1;
	curFrameVSMInfo.physPageRowShift = log2((float)physPagesXY.x());
	curFrameVSMInfo.physPoolSize = physPoolSize;
	curFrameVSMInfo.physPoolPages = physPagesXY;

	manager.setPoolTextureSize(physPoolSize);
	physPagePool = &manager.poolTexture;
	frameData = NULL;
}

bool VirtualShadowMapArray::isAllocated() const
{
	return physPagePool && frameData;
}

VirtualShadowMap* VirtualShadowMapArray::allocate()
{
	VirtualShadowMap* vsm = NULL;
	int vsmID = shadowMaps.size();
	if (vsmID < cachedShadowMaps.size()) {
		vsm = cachedShadowMaps[vsmID];
		vsm->shadowMap = NULL;
		vsm->vsmID = vsmID;
	}
	else {
		vsm = new VirtualShadowMap(vsmID);
		cachedShadowMaps.push_back(vsm);
	}
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

	frameData = &manager->getCurFrameData();

	vector<VirtualShadowMapPrevData> vsmPrevDataUpload;
	manager->buildPrevData(shadowMaps, vsmPrevDataUpload);
	vsmPrevData.uploadData(vsmPrevDataUpload.size(), vsmPrevDataUpload.data());

	frameData->projData.uploadData(shadowMapCount, projDataUpload.data());

	curFrameVSMInfo.numShadowMapSlots = shadowMapCount;
	frameData->vsmInfo.uploadData(1, &curFrameVSMInfo);

	const int pageFlagsCount = std::max(128 * 1024, shadowMapCount * int(VirtualShadowMapConfig::pageTableSize));
	pageRequestFlags.resize(pageFlagsCount);
	context.clearOutputBufferUint(pageRequestFlags.getVendorGPUBuffer(), Vector4u::Zero());

	// not use hzb
	//dirtyPageFlags.resize(curFrameVSMInfo.maxPhysPages);
	//context.clearOutputBufferUint(dirtyPageFlags.getVendorGPUBuffer(), Vector4u::Zero());

	const int pageRectCount = shadowMaps.size() * VirtualShadowMapConfig::maxMips;
	frameData->pageRect.resize(pageRectCount);
	context.bindShaderProgram(VirtualShadowMapShaders::initPageRectsProgram);
	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->pageRect.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageRectName, { true });
	context.dispatchCompute(ceil(pageRectCount / (float)VirtualShadowMapShaders::initPageRectsProgramDim.x()), 1, 1);

	GenPageFlagInfo genPageFlagInfo;
	genPageFlagInfo.directLightCount = true;
	genPageFlagInfo.pageDilationBorderSizeMain = config.pageDilationBorderSizeMain;
	genPageFlagInfo.pageDilationBorderSizeLocal = config.pageDilationBorderSizeLocal;
	genPageFlagInfo.backFaceCull = true;

	genPageFlagInfoBuffer.uploadData(1, &genPageFlagInfo, true);

	resizeDirectLightVSMIDs(cameraDatas);

	context.clearFrameBindings();

	if (config.debugViewMode == VirtualShadowMapConfig::ClipmapLevel)
		context.bindShaderProgram(VirtualShadowMapShaders::generatePageFlagsFromPixelsDebugProgram);
	else
		context.bindShaderProgram(VirtualShadowMapShaders::generatePageFlagsFromPixelsProgram);

	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
	context.bindBufferBase(genPageFlagInfoBuffer.getVendorGPUBuffer(), VirtualShadowMapShaders::GenPageFlagInfoName);
	context.bindBufferBase(pageRequestFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageRequestFlagsName, { true });

	Image debugImage;
	IDebugBufferGetter* debugGetter = dynamic_cast<IDebugBufferGetter*>((*cameraDatas.begin())->surfaceBuffer);
	debugImage.texture = debugGetter->getDebugBuffer();

	auto bindDebugImage = [&] {
		if (debugImage.texture)
			context.bindImage(debugImage, VirtualShadowMapShaders::outDebugBufferName);
	};

	int camIndex = 0;
	for (auto& cameraData : cameraDatas) {
		IGBufferGetter* getter = dynamic_cast<IGBufferGetter*>(cameraData->surfaceBuffer);
		Texture* sceneDepthMap = getter->getGBufferB();
		Texture* sceneNormalMap = getter->getGBufferC();

		if (sceneDepthMap == NULL || sceneNormalMap == NULL)
			throw runtime_error("SurfaceBuffer dose not have sceneDepthMap or sceneDepthMap");

		vector<unsigned int> directLightVSMIDsUpload;
		for (VirtualShadowMapClipmap* clipmap : lightData.mainLightClipmaps) {
			if (clipmap->getCameraRenderData() == cameraData) {
				directLightVSMIDsUpload.push_back(clipmap->getVirtualShadowMap(0)->vsmID);
			}
		}

		if (directLightVSMIDsUpload.empty())
			continue;

		if (cameraData->cameraRender->isMainCameraRender() && config.debugViewMode == VirtualShadowMapConfig::ClipmapLevel)
			bindDebugImage();

		GPUBuffer* curDirectLightVSMIDs = directLightVSMIDs[camIndex];

		curDirectLightVSMIDs->uploadData(directLightVSMIDsUpload.size(), directLightVSMIDsUpload.data(), true);

		cameraData->bind(context);
		context.bindBufferBase(curDirectLightVSMIDs->getVendorGPUBuffer(), VirtualShadowMapShaders::directLightVSMIDsName);
		context.bindTexture(sceneDepthMap->getVendorTexture(), VirtualShadowMapShaders::sceneDepthMapName);
		context.bindTexture(sceneNormalMap->getVendorTexture(), VirtualShadowMapShaders::sceneNormalMapName);
		context.dispatchCompute(
			ceil(cameraData->data.viewSize.x() / (float)VirtualShadowMapShaders::generatePageFlagsFromPixelsProgramDim.x()),
			ceil(cameraData->data.viewSize.y() / (float)VirtualShadowMapShaders::generatePageFlagsFromPixelsProgramDim.y()),
			1);
		camIndex++;
	}

	context.unbindBufferBase(VirtualShadowMapShaders::outPageRequestFlagsName);

	if (config.debugViewMode == VirtualShadowMapConfig::ClipmapLevel) {
		context.unbindBufferBase(VirtualShadowMapShaders::outDebugBufferName);
		return;
	}

	frameData->pageTable.resize(pageFlagsCount);
	frameData->pageFlags.resize(pageFlagsCount);

	// One additional element as the last element is used as an atomic counter
	freePhysPages.resize(curFrameVSMInfo.maxPhysPages + 2);
	physPageAllocRequests.resize(curFrameVSMInfo.maxPhysPages + 1);

	frameData->physPageMetaData.resize(curFrameVSMInfo.maxPhysPages);
	allocPageRect.resize(pageRectCount);

	context.bindShaderProgram(VirtualShadowMapShaders::initPhysicalPageMetaDataProgram);
	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageMetaDataName, { true });
	context.bindBufferBase(freePhysPages.getVendorGPUBuffer(), VirtualShadowMapShaders::outFreePhysPagesName, { true });
	context.bindBufferBase(physPageAllocRequests.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageAllocRequestsName, { true });
	context.dispatchCompute(ceil(curFrameVSMInfo.maxPhysPages / (float)VirtualShadowMapShaders::initPhysicalPageMetaDataProgramDim.x()), 1, 1);

	bool isCacheValid = manager->isCacheValid();
	context.bindShaderProgram(isCacheValid ?
		VirtualShadowMapShaders::createCachedPageMappingsHasCacheProgram :
		VirtualShadowMapShaders::createCachedPageMappingsNoCacheProgram);
	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
	context.bindBufferBase(vsmPrevData.getVendorGPUBuffer(), VirtualShadowMapShaders::vsmPrevDataName);
	context.bindBufferBase(pageRequestFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::pageRequestFlagsName);
	manager->bindPrevFrameData(context);
	context.bindBufferBase(frameData->pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageTableName, { true });
	context.bindBufferBase(frameData->pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageFlagsName, { true });
	context.bindBufferBase(frameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageMetaDataName, { true });
	context.bindBufferBase(physPageAllocRequests.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageAllocRequestsName, { true });
	context.dispatchCompute(ceil(config.pageTableSize / (float)(isCacheValid ?
			VirtualShadowMapShaders::createCachedPageMappingsHasCacheProgramDim.x() :
			VirtualShadowMapShaders::createCachedPageMappingsNoCacheProgramDim.x())), shadowMapCount, 1);

	context.unbindBufferBase(VirtualShadowMapShaders::outPhysPageMetaDataName);
	context.unbindBufferBase(VirtualShadowMapShaders::outPhysPageAllocRequestsName);

	context.bindShaderProgram(VirtualShadowMapShaders::packFreePagesProgram);
	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageMetaDataName);
	context.bindBufferBase(freePhysPages.getVendorGPUBuffer(), VirtualShadowMapShaders::outFreePhysPagesName, { true });
	context.dispatchCompute(ceil(curFrameVSMInfo.maxPhysPages / (float)VirtualShadowMapShaders::packFreePagesProgramDim.x()), 1, 1);

	context.unbindBufferBase(VirtualShadowMapShaders::physPageMetaDataName);

	context.bindShaderProgram(VirtualShadowMapShaders::allocateNewPageMappingsProgram);
	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
	context.bindBufferBase(pageRequestFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::pageRequestFlagsName);
	context.bindBufferBase(physPageAllocRequests.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageAllocRequestsName);
	context.bindBufferBase(frameData->pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageTableName, { true });
	context.bindBufferBase(frameData->pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageFlagsName, { true });
	context.bindBufferBase(freePhysPages.getVendorGPUBuffer(), VirtualShadowMapShaders::outFreePhysPagesName, { true });
	context.bindBufferBase(frameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPageMetaDataName, { true });
	context.dispatchCompute(ceil(curFrameVSMInfo.maxPhysPages / (float)VirtualShadowMapShaders::allocateNewPageMappingsProgramDim.x()), 1, 1);

	context.unbindBufferBase(VirtualShadowMapShaders::outPageFlagsName);
	context.unbindBufferBase(VirtualShadowMapShaders::outPhysPageMetaDataName);

	context.bindShaderProgram(VirtualShadowMapShaders::generateHierarchicalPageFlagsProgram);
	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageMetaDataName);
	context.bindBufferBase(frameData->pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageFlagsName, { true });
	context.bindBufferBase(frameData->pageRect.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageRectName, { true });
	context.dispatchCompute(ceil(config.pageTableSize / (float)VirtualShadowMapShaders::generateHierarchicalPageFlagsProgramDim.x()), shadowMapCount, 1);

	{
		// Skip PropagateMappedMips, for only having clipmaps
	}

	initPhysPagesIndirectArgs.resize(3);

	context.bindShaderProgram(VirtualShadowMapShaders::clearInitPhysPagesIndirectArgsProgram);
	context.bindBufferBase(initPhysPagesIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outInitPhysPagesIndirectArgsName, { true });
	context.dispatchCompute(1, 1, 1);

	physPagesToInit.resize(curFrameVSMInfo.maxPhysPages + 1);

	context.bindShaderProgram(VirtualShadowMapShaders::selectPagesToInitProgram);
	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageMetaDataName);
	context.bindBufferBase(initPhysPagesIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outInitPhysPagesIndirectArgsName, { true });
	context.bindBufferBase(physPagesToInit.getVendorGPUBuffer(), VirtualShadowMapShaders::outPhysPagesToInitName, { true });
	context.dispatchCompute(ceil(curFrameVSMInfo.maxPhysPages / (float)VirtualShadowMapShaders::selectPagesToInitProgramDim.x()), 1, 1);

	context.unbindBufferBase(VirtualShadowMapShaders::outPhysPagesToInitName);

	context.bindShaderProgram(VirtualShadowMapShaders::initPhysicalMemoryIndirectProgram);
	context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	context.bindBufferBase(frameData->physPageMetaData.getVendorGPUBuffer(), VirtualShadowMapShaders::physPageMetaDataName);
	context.bindBufferBase(physPagesToInit.getVendorGPUBuffer(), VirtualShadowMapShaders::physPagesToInitName);
	Image image;
	image.texture = physPagePool;
	context.bindImage(image, VirtualShadowMapShaders::outPhysPagePoolName);
	context.dispatchComputeIndirect(initPhysPagesIndirectArgs.getVendorGPUBuffer(), 0);

	context.clearOutputBufferBindings();
}

void VirtualShadowMapArray::render(IRenderContext& context, const LightRenderData& lightData)
{
	VirtualShadowMapConfig& config = VirtualShadowMapConfig::instance();
	if (config.debugViewMode == VirtualShadowMapConfig::ClipmapLevel)
		return;
	cullingPasses(context, lightData);
	VSMInstanceDrawResource resourceContext;
	for (int i = 0; i < cullingBatchInfos.size(); i++) {
		CullingBatchInfo* batchInfo = cullingBatchInfos[i];
		VirtualShadowMapManager::LightEntry::Batch* batch = batchInfo->batch;
		int cmdCount = batch->indirectCommands.size();
		for (int cmdIndex = 0; cmdIndex < cmdCount; cmdIndex++) {
			VSMInstanceDrawResource& resource = batch->resources[cmdIndex];
			bool shaderSwitch = false;

			if (resourceContext.shaderProgram != resource.shaderProgram) {
				resourceContext.shaderProgram = resource.shaderProgram;
				shaderSwitch = true;

				context.bindShaderProgram(resource.shaderProgram);
			}

			if (shaderSwitch || resourceContext.transformData != resource.transformData) {
				resourceContext.transformData = resource.transformData;

				resource.transformData->bind(context);
			}

			context.setRenderOpaqueState();

			for (auto data : resource.extraData) {
				data->bind(context);
			}

			if (shaderSwitch || resourceContext.meshData != resource.meshData) {
				resourceContext.meshData = resource.meshData;

				context.bindMeshData(resource.meshData);
			}

			if (shaderSwitch || resourceContext.materialData != resource.materialData) {
				resourceContext.materialData = resource.materialData;

				resource.materialData->bind(context);
			}

			if (shaderSwitch) {
				context.bindBufferBase(batchInfo->instanceIDs.getVendorGPUBuffer(), VirtualShadowMapShaders::instanceIDsName);
				context.bindBufferBase(batchInfo->pageInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::pageInfoName);
				context.bindBufferBase(frameData->pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::pageTableName);
				context.bindBufferBase(frameData->pageRect.getVendorGPUBuffer(), VirtualShadowMapShaders::pageRectName);
				context.bindBufferBase(shadowViewInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::shadowViewInfosName);
			}

			BufferOption option;
			option.output = false;
			option.offset = cmdIndex * batchInfo->shadowDepthInstanceOffset.getVendorGPUBuffer()->desc.cellSize;
			option.stride = 0;
			context.bindBufferBase(batchInfo->shadowDepthInstanceOffset.getVendorGPUBuffer(), 0, option);

			Image image;
			image.texture = physPagePool;
			context.bindImage(image, VirtualShadowMapShaders::outPhysPagePoolName);

			context.setViewport(0, 0, VirtualShadowMapConfig::virtualShadowMapSize, VirtualShadowMapConfig::virtualShadowMapSize);

			context.drawMeshIndirect(batchInfo->shadowDepthIndirectArgs.getVendorGPUBuffer(), cmdIndex * sizeof(DrawElementsIndirectCommand));
		}
	}
	context.clearOutputBufferBindings();
}

void VirtualShadowMapArray::renderDebugView(IRenderContext& context, const CameraRenderData& mainCameraData)
{
	VirtualShadowMapConfig& config = VirtualShadowMapConfig::instance();
	if (config.debugViewMode == VirtualShadowMapConfig::ClipmapLevel) {
		IGBufferGetter* gBufferGetter = dynamic_cast<IGBufferGetter*>(mainCameraData.surfaceBuffer);
		IDebugBufferGetter* debugBufferGetter = dynamic_cast<IDebugBufferGetter*>(mainCameraData.surfaceBuffer);
		Texture* sceneColor = gBufferGetter->getGBufferA();
		Texture* debugBuffer = debugBufferGetter->getDebugBuffer();
		if (sceneColor == NULL || debugBuffer == NULL)
			return;

		static ShaderPropertyName outBufferName = "outBuffer";

		context.bindShaderProgram(VirtualShadowMapShaders::debugBlitProgram);
		context.bindTexture(debugBuffer->getVendorTexture(), VirtualShadowMapShaders::debugBufferName);
		Image image;
		image.texture = sceneColor;
		context.bindImage(image, outBufferName);
		context.dispatchCompute(
			ceil(mainCameraData.data.viewSize.x() / (float)VirtualShadowMapShaders::debugBlitProgramDim.x()),
			ceil(mainCameraData.data.viewSize.y() / (float)VirtualShadowMapShaders::debugBlitProgramDim.y()),
			1);
		context.unbindBufferBase(outBufferName);
	}

	int debugVSMID = VirtualShadowMapConfig::instance().debugVSMID;
	if (config.debugViewMode == VirtualShadowMapConfig::Clipmap && debugVSMID >= 0) {
		context.bindShaderProgram(VirtualShadowMapShaders::debugClipmapProgram);
		context.setDrawInfo(debugVSMID, 0, 0);
		Image image;
		image.texture = &clipmapDebugBuffer;
		context.bindImage(image, VirtualShadowMapShaders::outDebugBufferName);
		context.bindTexture(physPagePool->getVendorTexture(), VirtualShadowMapShaders::physPagePoolName);
		context.bindBufferBase(frameData->projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
		context.bindBufferBase(frameData->pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::pageTableName);
		context.bindBufferBase(frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
		context.dispatchCompute(
			ceil(VirtualShadowMapConfig::virtualShadowMapSize / (float)VirtualShadowMapShaders::debugClipmapProgramDim.x()),
			ceil(VirtualShadowMapConfig::virtualShadowMapSize / (float)VirtualShadowMapShaders::debugClipmapProgramDim.y()),
			1);
		context.unbindBufferBase(VirtualShadowMapShaders::outDebugBufferName);
	}
}

void VirtualShadowMapArray::clean()
{
	shadowMaps.clear();
	cullingBatchInfos.clear();
	shadowViewInfosUpload.clear();
}

Texture2D& VirtualShadowMapArray::getClipmapDebugBuffer()
{
	return clipmapDebugBuffer;
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
	info->data.maxPerInstanceCmdCount = 0;
	info->data.frame = Time::frames();
	cullingBatchInfos.push_back(info);
	return info;
}

void VirtualShadowMapArray::buildCullingBatchInfos(const LightRenderData& lightData)
{
	for (VirtualShadowMapClipmap* clipmap : lightData.mainLightClipmaps) {
		unsigned int firstView = shadowViewInfosUpload.size();
		unsigned int viewCount = 0;
		for (int levelIndex = 0; levelIndex < clipmap->getLevelCount(); levelIndex++) {
			ShadowViewInfo view;
			if (clipmap->getLevelShadowViewInfo(levelIndex, view)) {
				shadowViewInfosUpload.push_back(view);
				viewCount++;
			}
		}

		VirtualShadowMapManager::LightEntry* lightEntry = clipmap->getLightEntry();
		if (lightEntry) {
			lightEntry->markRendered(Time::frames());
			for (const VirtualShadowMapManager::LightEntry::Batch& batch : lightEntry->batches) {
				CullingBatchInfo* cullingBatchInfo = fetchCullingBatchInfo();
				cullingBatchInfo->batch = (VirtualShadowMapManager::LightEntry::Batch*)&batch;
				cullingBatchInfo->data.firstView = firstView;
				cullingBatchInfo->data.viewCount = viewCount;
				int drawInstanceCount = batch.drawInstanceInfos.size();
				int indirectDrawCount = batch.indirectCommands.size();
				int maxInstanceDrawPerPass = drawInstanceCount * VirtualShadowMapConfig::maxPerInstanceCmdCount;
				cullingBatchInfo->data.maxPerInstanceCmdCount = maxInstanceDrawPerPass;
				cullingBatchInfo->data.instanceCount = drawInstanceCount;
				cullingBatchInfo->drawInstanceInfos.uploadData(cullingBatchInfo->data.instanceCount,
					(void*)batch.drawInstanceInfos.data());
				cullingBatchInfo->shadowDepthIndirectArgs.uploadData(
					batch.indirectCommands.size() * (sizeof(DrawElementsIndirectCommand) / sizeof(unsigned int)),
					(void*)batch.indirectCommands.data());
				AllocCmdInfo allocCmdInfoUpload;
				allocCmdInfoUpload.indirectArgCount = indirectDrawCount;
				cullingBatchInfo->allocCmdInfo.uploadData(1, &allocCmdInfoUpload);
				cullingBatchInfo->visiableInstanceInfos.resize(maxInstanceDrawPerPass);
				cullingBatchInfo->visiableInstanceCount.resize(1);
				cullingBatchInfo->offsetBufferCount.resize(1);
				cullingBatchInfo->outputCommandListsIndirectArgs.resize(3);
				cullingBatchInfo->shadowDepthInstanceOffset.resize(indirectDrawCount);
				cullingBatchInfo->shadowDepthInstanceCounter.resize(indirectDrawCount);
				cullingBatchInfo->instanceIDs.resize(maxInstanceDrawPerPass);
				cullingBatchInfo->pageInfo.resize(maxInstanceDrawPerPass);
				cullingBatchInfo->cullingData.uploadData(1, &cullingBatchInfo->data, true);
			}
		}
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
		cullingBatchInfo->batch->transformData->bind(context);
		context.bindBufferBase(frameData->projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
		context.bindBufferBase(frameData->pageFlags.getVendorGPUBuffer(), VirtualShadowMapShaders::pageFlagsName);
		context.bindBufferBase(frameData->pageRect.getVendorGPUBuffer(), VirtualShadowMapShaders::pageRectName);
		context.bindBufferBase(shadowViewInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::shadowViewInfosName);
		context.bindBufferBase(cullingBatchInfo->cullingData.getVendorGPUBuffer(), VirtualShadowMapShaders::CullingDataName);
		context.bindBufferBase(cullingBatchInfo->drawInstanceInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::drawInstanceInfosName);
		context.bindBufferBase(cullingBatchInfo->visiableInstanceInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::outVisiableInstanceInfosName, { true });
		context.bindBufferBase(cullingBatchInfo->visiableInstanceCount.getVendorGPUBuffer(), VirtualShadowMapShaders::outVisiableInstanceCountName, { true });
		context.bindBufferBase(cullingBatchInfo->shadowDepthIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outShadowDepthIndirectArgsName, { true });
		context.dispatchCompute(ceil(indirectDrawCount / (float)VirtualShadowMapShaders::cullPerPageDrawCommandsProgramDim.x()), 1, 1);

		context.unbindBufferBase(VirtualShadowMapShaders::outVisiableInstanceInfosName);
		context.unbindBufferBase(VirtualShadowMapShaders::outVisiableInstanceCountName);
		context.unbindBufferBase(VirtualShadowMapShaders::outShadowDepthIndirectArgsName);

		context.bindShaderProgram(VirtualShadowMapShaders::allocateCommandInstanceOutputSpaceProgram);
		context.clearOutputBufferUint(cullingBatchInfo->offsetBufferCount.getVendorGPUBuffer(), Vector4u::Zero());
		context.bindBufferBase(cullingBatchInfo->allocCmdInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::AllocCmdInfoName);
		context.bindBufferBase(cullingBatchInfo->shadowDepthIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::shadowDepthIndirectArgsName);
		context.bindBufferBase(cullingBatchInfo->offsetBufferCount.getVendorGPUBuffer(), VirtualShadowMapShaders::outOffsetBufferCountName, { true });
		context.bindBufferBase(cullingBatchInfo->shadowDepthInstanceOffset.getVendorGPUBuffer(), VirtualShadowMapShaders::outShadowDepthInstanceOffsetName, { true });
		context.bindBufferBase(cullingBatchInfo->shadowDepthInstanceCounter.getVendorGPUBuffer(), VirtualShadowMapShaders::shadowDepthInstanceCounterName, { true });
		context.dispatchCompute(ceil(indirectDrawCount / (float)VirtualShadowMapShaders::allocateCommandInstanceOutputSpaceProgramDim.x()), 1, 1);

		context.unbindBufferBase(VirtualShadowMapShaders::outOffsetBufferCountName);
		context.unbindBufferBase(VirtualShadowMapShaders::shadowDepthInstanceCounterName);

		context.bindShaderProgram(VirtualShadowMapShaders::initOutputommandInstanceListsArgsProgram);
		context.bindBufferBase(cullingBatchInfo->offsetBufferCount.getVendorGPUBuffer(), VirtualShadowMapShaders::offsetBufferCountName);
		context.bindBufferBase(cullingBatchInfo->outputCommandListsIndirectArgs.getVendorGPUBuffer(), VirtualShadowMapShaders::outputCommandListsIndirectArgsName, { true });
		context.dispatchCompute(1, 1, 1);

		context.unbindBufferBase(VirtualShadowMapShaders::outShadowDepthInstanceOffsetName);

		context.bindShaderProgram(VirtualShadowMapShaders::outputCommandInstanceListsProgram);
		context.bindBufferBase(cullingBatchInfo->visiableInstanceInfos.getVendorGPUBuffer(), VirtualShadowMapShaders::visiableInstanceInfosName);
		context.bindBufferBase(cullingBatchInfo->visiableInstanceCount.getVendorGPUBuffer(), VirtualShadowMapShaders::visiableInstanceCountName);
		context.bindBufferBase(cullingBatchInfo->shadowDepthInstanceCounter.getVendorGPUBuffer(), VirtualShadowMapShaders::shadowDepthInstanceCounterName, { true });
		context.bindBufferBase(cullingBatchInfo->instanceIDs.getVendorGPUBuffer(), VirtualShadowMapShaders::outInstanceIDsName, { true });
		context.bindBufferBase(cullingBatchInfo->pageInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::outPageInfoName, { true });
		context.dispatchComputeIndirect(cullingBatchInfo->outputCommandListsIndirectArgs.getVendorGPUBuffer(), 0);
	}
	context.clearOutputBufferBindings();
}
