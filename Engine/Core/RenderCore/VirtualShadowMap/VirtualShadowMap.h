#pragma once
#include "../RenderInterface.h"
#include "../LightRenderData.h"
#include "../MeshBatchRenderData.h"

struct DebugRenderData;

constexpr unsigned int log2Const(unsigned int n)
{
	return (n > 1) ? 1 + log2Const(n / 2) : 0;
}

constexpr unsigned int calVirtualShadowMapLevelOffsets(unsigned int level, unsigned int log2Level0Pages)
{
	unsigned int NumBits = level << 1;
	unsigned int StartBit = (2U * log2Level0Pages + 2U) - NumBits;
	unsigned int Mask = ((1U << NumBits) - 1U) << StartBit;
	return 0x55555555U & Mask;
}

enum { VSM_None_ID = -1 };

enum VSM_LightType
{
	VSM_DirectLight,
	VSM_LocalLight,
};

enum struct VSM_DebugViewFlag
{
	None,
	GenPageFlagsFromPixels
};

class VirtualShadowMapConfig : public Serializable
{
public:
	Serialize(VirtualShadowMapConfig, );

	static constexpr unsigned int pageSize = 128;
	static constexpr unsigned int log2PageSize = log2Const(pageSize);
	static constexpr unsigned int level0Pages = 128;
	static constexpr unsigned int log2Level0Pages = log2Const(level0Pages);
	static constexpr unsigned int maxMips = log2Level0Pages + 1;

	static constexpr unsigned int pageTableSize = calVirtualShadowMapLevelOffsets(maxMips, log2Level0Pages);

	static constexpr unsigned int virtualShadowMapSize = pageSize * pageSize;

	static constexpr unsigned int maxPerInstanceCmdCount = 64;

	bool enable = false;
	bool useStaticCache = false;
	unsigned int physPoolWidth = 128 * 128;
	unsigned int maxPhysPages = 4096;
	unsigned int firstClipmapLevel = 6;
	unsigned int lastClipmapLevel = 16;

	float clipmapRadiusZScale = 1000.0f;
	float resolutionLodBiasClipmap = -0.5f;
	float resolutionLodBiasLocal = -0.5f;

	float pageDilationBorderSizeMain = 0.05f;
	float pageDilationBorderSizeLocal = 0.05f;

	float localLightViewMinZ = 0.001f;
	float screenRayLength = 0.0005f;

	unsigned int pcfPixel = 2;
	unsigned int pcfStep = 1;
	float pcfRadiusScale = 1;

	enum DebugViewMode
	{
		None,
		ClipmapLevel,
		Clipmap,
		Num
	};

	int debugViewMode = DebugViewMode::None;
	int debugVSMID = -1;

	Vector2u getPhysPagesXY() const;

	static VirtualShadowMapConfig& instance();
	static void setEnable(bool enable);
	static bool isEnable();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

struct VirtualShadowMapProjectionData
{
	Matrix4f viewOriginToView;
	Matrix4f viewToClip;
	Matrix4f viewOriginToClip;
	Matrix4f viewOriginToUV;

	Vector3f clipmapWorldOrigin;
	int resolutionLodBias;
	
	Vector2i clipmapOffset;
	int clipmapIndex;
	int clipmapLevel;

	Vector3f worldCenter;
	int clipmapCount;

	Vector3f worldDirection;
	int vsmID;

	int lightType;

	int uncached;

	float screenRayLength;

	int pad;
};

struct VirtualShadowMapInfo
{
	unsigned int physPageRowMask;
	unsigned int physPageRowShift;
	unsigned int maxPhysPages;
	unsigned int numShadowMapSlots;
	Vector2u physPoolSize;
	Vector2u physPoolPages;
	unsigned int staticPageIndex;
	unsigned int pcfPixel;
	unsigned int pcfStep;
	float pcfRadiusScale;
	unsigned int flag;
	unsigned int pad[3];
};

struct VirtualShadowMapPhysicalPageData
{
	unsigned int flag;
	unsigned int age;
	unsigned int virtualPageOffset;
	unsigned int vsmID;
};

struct VirtualShadowMapPhysicalPageRequest
{
	unsigned int vsmID;
	unsigned int globalPageOffset;
};

struct VirtualShadowMapPrevData
{
	int vsmID = VSM_None_ID;
	Vector2i ClipmapCornerOffsetDelta;
	int pad;
};

struct VirtualShadowMapFrameData
{
	GPUBuffer vsmInfo;
	GPUBuffer pageTable;
	GPUBuffer pageFlags;
	GPUBuffer projData;
	GPUBuffer pageRect;
	GPUBuffer physPageMetaData;

	VirtualShadowMapFrameData();
	~VirtualShadowMapFrameData();

	void release();
};

class VirtualShadowMapShaders
{
public:
#define VSM_SHADER(name) \
static ShaderProgram* name##Program; \
static ShaderProgram* name##DebugProgram; \
static Vector3u name##ProgramDim;

	VSM_SHADER(procInvalidations);
	VSM_SHADER(initPageRects);
	VSM_SHADER(generatePageFlagsFromPixels);
	VSM_SHADER(initPhysicalPageMetaData);
	VSM_SHADER(createCachedPageMappingsHasCache);
	VSM_SHADER(createCachedPageMappingsNoCache);
	VSM_SHADER(packFreePages);
	VSM_SHADER(sortFreePages);
	VSM_SHADER(allocateNewPageMappings);
	VSM_SHADER(generateHierarchicalPageFlags);
	VSM_SHADER(propagateMappedMips);
	VSM_SHADER(clearComputeIndirectArgs);
	VSM_SHADER(selectPagesToInit);
	VSM_SHADER(initPhysicalMemoryIndirect);
	VSM_SHADER(selectPagesToMerge);
	VSM_SHADER(mergePhysicalPagesIndirect);
	VSM_SHADER(cullPerPageDrawCommands);
	VSM_SHADER(allocateCommandInstanceOutputSpace);
	VSM_SHADER(initOutputommandInstanceListsArgs);
	VSM_SHADER(outputCommandInstanceLists);
	VSM_SHADER(debugBlit);
	VSM_SHADER(debugClipmap);

	static const ShaderPropertyName VSMInfoBuffName;
	static const ShaderPropertyName vsmPrevDataName;
	static const ShaderPropertyName pageTableName;
	static const ShaderPropertyName pageFlagsName;
	static const ShaderPropertyName pageRectName;
	static const ShaderPropertyName physPageMetaDataName;
	static const ShaderPropertyName freePhysPagesName;
	static const ShaderPropertyName projDataName;
	static const ShaderPropertyName pageRequestFlagsName;
	static const ShaderPropertyName physPageAllocRequestsName;
	static const ShaderPropertyName invalidationIndicesName;
	static const ShaderPropertyName invalidationInfoName;
	static const ShaderPropertyName outPrevPhysPageMetaDataName;
	static const ShaderPropertyName outPageRequestFlagsName;
	static const ShaderPropertyName GenPageFlagInfoName;
	static const ShaderPropertyName directLightVSMIDsName;
	static const ShaderPropertyName sceneDepthMapName;
	static const ShaderPropertyName sceneNormalMapName;
	static const ShaderPropertyName prevPageTableName;
	static const ShaderPropertyName prevPageFlagsName;
	static const ShaderPropertyName prevPhysPageMetaDataName;
	static const ShaderPropertyName outPageTableName;
	static const ShaderPropertyName outPageFlagsName;
	static const ShaderPropertyName outPageRectName;
	static const ShaderPropertyName outPhysPageMetaDataName;
	static const ShaderPropertyName outFreePhysPagesName;
	static const ShaderPropertyName outPhysPageAllocRequestsName;

	static const ShaderPropertyName outInitPhysPagesIndirectArgsName;
	static const ShaderPropertyName physPagesToInitName;
	static const ShaderPropertyName outPhysPagesToInitName;

	static const ShaderPropertyName outMergePhysPagesIndirectArgsName;
	static const ShaderPropertyName physPagesToMergeName;
	static const ShaderPropertyName outPhysPagesToMergeName;

	static const ShaderPropertyName outComputeIndirectArgsName;

	static const ShaderPropertyName physPagePoolName;
	static const ShaderPropertyName outPhysPagePoolName;

	static const ShaderPropertyName CullingDataName;
	static const ShaderPropertyName AllocCmdInfoName;

	static const ShaderPropertyName shadowViewInfosName;

	static const ShaderPropertyName drawInstanceInfosName;
	static const ShaderPropertyName visiableInstanceInfosName;
	static const ShaderPropertyName visiableInstanceCountName;
	static const ShaderPropertyName offsetBufferCountName;
	static const ShaderPropertyName shadowDepthIndirectArgsName;

	static const ShaderPropertyName shadowDepthInstanceCounterName;

	static const ShaderPropertyName outVisiableInstanceInfosName;
	static const ShaderPropertyName outVisiableInstanceCountName;
	static const ShaderPropertyName outOffsetBufferCountName;
	static const ShaderPropertyName outShadowDepthInstanceOffsetName;

	static const ShaderPropertyName outputCommandListsIndirectArgsName;
	static const ShaderPropertyName outShadowDepthIndirectArgsName;

	static const ShaderPropertyName instanceIDsName;
	static const ShaderPropertyName outInstanceIDsName;

	static const ShaderPropertyName pageInfosName;
	static const ShaderPropertyName outPageInfosName;

	static const ShaderPropertyName debugBufferName;
	static const ShaderPropertyName outDebugBufferName;

	static void loadDefaultResource();
};

class VirtualShadowMap;
class VirtualShadowMapArray;

struct VSMDrawInstanceInfo
{
	unsigned int instanceID;
	unsigned int indirectArgIndex;
};

struct VSMInstanceDrawResource
{
	struct ExecutionOrder
	{
		bool operator()(const VSMInstanceDrawResource& t0, const VSMInstanceDrawResource& t1) const;
		bool operator()(const VSMInstanceDrawResource* t0, const VSMInstanceDrawResource* t1) const;
	};

	bool reverseCullMode = false;
	IRenderData* transformData = NULL;
	MaterialRenderData* materialData = NULL;
	ShaderProgram* shaderProgram = NULL;
	MeshData* meshData = NULL;
	list<IRenderData*> extraData;
};

template<class Data>
struct TVSMMeshBatchDrawCall : TDrawCallBase<Data>
{
	vector<Data> instanceData;
	DrawElementsIndirectCommand command;
	unsigned int drawCommandOffset = 0;

	bool reverseCullMode = false;
	IRenderData* transformData = NULL;
	ShaderProgram* shaderProgram = NULL;
	list<IRenderData*> bindings;
	
	template<class K>
	void addDraw(const K& key, const Data& data, unsigned int instanceIndex, unsigned int instanceCount);

	void calculateCountAndOffset(unsigned int instanceOffset, unsigned int commandOffset);
	
	void fetchInstanceData(Data* dst, unsigned int count);
	unsigned int getInstanceCount() const;
	unsigned int getInstanceOffset() const;

	void fetchDrawCommands(DrawElementsIndirectCommand* dst, unsigned int count);
	unsigned int getDrawCommandCount() const;
	unsigned int getDrawCommandOffset() const;
	
	void clean();
};

template <class Data>
template <class K>
void TVSMMeshBatchDrawCall<Data>::addDraw(const K& key, const Data& data, unsigned int instanceIndex, unsigned int instanceCount)
{
	if (command.instanceCount == 0)
		key.assignDrawCommand(command);
	int index = command.instanceCount;
	int count = index + instanceCount;
	if (instanceData.size() < count)
		instanceData.resize(count);
	Data _data = data;
	for (; index < count; index++) {
		instanceData[index] = _data;
		_data = drawInstanceIncrease(_data);
	}
	command.instanceCount += instanceCount;
}

template <class Data>
void TVSMMeshBatchDrawCall<Data>::calculateCountAndOffset(unsigned int instanceOffset, unsigned int commandOffset)
{
	command.baseInstance = instanceOffset;
	drawCommandOffset = commandOffset;
}

template <class Data>
void TVSMMeshBatchDrawCall<Data>::fetchInstanceData(Data* dst, unsigned int count)
{
	memcpy(dst, instanceData.data(), sizeof(Data) * command.instanceCount);
}

template <class Data>
unsigned TVSMMeshBatchDrawCall<Data>::getInstanceCount() const
{
	return command.instanceCount;
}

template <class Data>
unsigned TVSMMeshBatchDrawCall<Data>::getInstanceOffset() const
{
	return command.baseInstance;
}

template <class Data>
void TVSMMeshBatchDrawCall<Data>::fetchDrawCommands(DrawElementsIndirectCommand* dst, unsigned int count)
{
	memcpy(dst, &command, sizeof(DrawElementsIndirectCommand));
}

template <class Data>
unsigned TVSMMeshBatchDrawCall<Data>::getDrawCommandCount() const
{
	return 1;
}

template <class Data>
unsigned TVSMMeshBatchDrawCall<Data>::getDrawCommandOffset() const
{
	return drawCommandOffset;
}

template <class Data>
void TVSMMeshBatchDrawCall<Data>::clean()
{
	command.baseInstance = 0;
	command.instanceCount = 0;
	drawCommandOffset = 0;
	
	reverseCullMode = false;
	transformData = NULL;
	shaderProgram = NULL;
	bindings.clear();
}

struct VSMMeshBatchDrawKey : MeshBatchDrawKey
{
	VSMMeshBatchDrawKey(MeshPart* meshPart, Material* material, bool negativeScale = false);
	bool isValid() const;
	bool operator<(const VSMMeshBatchDrawKey& key) const;
	bool operator==(const VSMMeshBatchDrawKey& key) const;
};

template <>
struct std::hash<VSMMeshBatchDrawKey>
{
	std::size_t operator()(const VSMMeshBatchDrawKey& key) const;
};

typedef TVSMMeshBatchDrawCall<InstanceDrawData> VSMMeshBatchDrawCall;
typedef TMeshBatchDrawCallCollection<VSMMeshBatchDrawKey, InstanceDrawData, TVSMMeshBatchDrawCall> VSMMeshBatchDrawCallCollection;

class VirtualShadowMapManager
{
public:
	struct LightEntry;

	struct ShadowMap
	{
		int preVirtualShadowMapID = VSM_None_ID;
		int curVirtualShadowMapID = VSM_None_ID;

		Vector2i prePageOffset;
		Vector2i curPageOffset;

		struct Clipmap
		{
			Matrix4f worldToLightViewMatrix;
			float viewCenterZ;
			float viewRadiusZ;

			Vector2i preCornerOffset;
			Vector2i curCornerOffset;
		};

		Clipmap clipmap;

		bool isValid() const;

		void updateClipMap(
			int vsmID,
			const Matrix4f& worldToLightViewMatrix,
			const Vector2i& pageOffset,
			const Vector2i& cornerOffset,
			float levelRadius,
			float viewCenterZ,
			float ViewRadiusZ,
			const LightEntry& lightEntry);

		void updateLocal(int vsmID, const LightEntry& lightEntry);
	};

	struct LightEntry
	{
		struct Batch
		{
			struct Hasher
			{
				size_t operator()(const Batch& t) const { return (size_t)t.transformData; }
				size_t operator()(const Batch* t) const { return (size_t)t->transformData; }
			};

			IRenderData* transformData;
			vector<VSMDrawInstanceInfo> drawInstanceInfos;
			vector<DrawElementsIndirectCommand> indirectCommands;
			vector<VSMInstanceDrawResource> resources;

			bool operator==(const Batch& batch) const { return transformData == batch.transformData; }
		};
		struct LocalLightKey
		{
			Vector4f sphere;

			LocalLightKey();

			bool validateAndUpdate(const LocalLightData& lightData);
		};
		int lightID = -1;
		int preRenderFrame = -1;
		int curRenderFrame = -1;
		LocalLightKey localLightKey;
		vector<ShadowMap*> shadowMaps;
		unordered_set<Batch, Batch::Hasher> batches;

		ShadowMap* setShadowMap(int index);

		void updateClipmap();
		void updateLocal(const LocalLightData& lightData);
		void addMeshCommand(const VSMMeshBatchDrawCallCollection::CallItem& callItem);
		void markRendered(unsigned int frame);

		void invalidate();

		void clean();
	};

	struct InvalidationInfo
	{
		int numUpdateInstances;
		int pad[3];
	};
	GPUBuffer invalidationInfoBuffer;

	Texture2D poolTexture;
	vector<ShadowMap*> pages;

	VirtualShadowMapManager();

	bool isCacheValid() const;

	VirtualShadowMapFrameData& getCurFrameData();
	int getShadowMapCount() const;

	void invalidate();
	void setPoolTextureSize(Vector2u size, unsigned int arrayCount);
	LightEntry* setLightEntry(int lightID, int cameraID);

	void prepare();

	void buildPrevData(const vector<VirtualShadowMap*>& shadowMaps, vector<VirtualShadowMapPrevData>& prevData);
	void bindPrevFrameData(IRenderContext& context);

	void processInvalidations(IRenderContext& context, MeshTransformRenderData& transformRenderData);

	void swapFrameData(VirtualShadowMapArray& virtualShadowMapArray);
protected:
	unsigned int maxPageCount;
	unordered_map<unsigned long long, LightEntry*> preShadowMaps;
	unordered_map<unsigned long long, LightEntry*> curShadowMaps;
	VirtualShadowMapFrameData frameData[2];
	VirtualShadowMapFrameData* curFrameData;
	VirtualShadowMapFrameData* prevFrameData;
	VirtualShadowMapInfo prevShadowMapInfo;
};

struct VirtualShadowMapLightEntry : VirtualShadowMapManager::LightEntry {};

class VirtualShadowMap
{
public:
	int vsmID = VSM_None_ID;
	VirtualShadowMapManager::ShadowMap* shadowMap = NULL;

	VirtualShadowMap(int vsmID);
};

struct DirectShadowRenderCommand;

class VirtualShadowMapArray
{
public:
	VirtualShadowMapManager* manager;

	Texture2D* physPagePool;

	struct GenPageFlagInfo
	{
		int directLightCount;
		float pageDilationBorderSizeMain;
		float pageDilationBorderSizeLocal;
		int backFaceCull;
	};
	GPUBuffer genPageFlagInfoBuffer;

	VirtualShadowMapInfo curFrameVSMInfo;

	VirtualShadowMapFrameData* frameData = NULL;

	GPUBuffer vsmPrevData;
	GPUBuffer pageRequestFlags;
	vector<GPUBuffer*> directLightVSMIDs;
	GPUBuffer tempFreePhysPages;
	GPUBuffer freePhysPages;
	GPUBuffer physPageAllocRequests;
	GPUBuffer allocPageRect;

	GPUBuffer initPhysPagesIndirectArgs;
	GPUBuffer physPagesToInit;

	GPUBuffer mergePhysPagesIndirectArgs;
	GPUBuffer physPagesToMerge;

	struct CullingData
	{
		unsigned int instanceCount;
		unsigned int firstView;
		unsigned int viewCount;
		unsigned int primaryViewCount;
		unsigned int maxPerInstanceCmdCount;
		unsigned int frame;
		unsigned int pad[2];
	};

	struct VisiableInstanceInfo
	{
		unsigned int pageInfo;
		VSMDrawInstanceInfo drawInfo;
	};

	struct AllocCmdInfo
	{
		unsigned int indirectArgCount;
		unsigned int pad[3];
	};

	struct CullingBatchInfo
	{
		VirtualShadowMapManager::LightEntry::Batch* batch;
		CullingData data;
		GPUBuffer cullingData;
		GPUBuffer allocCmdInfo;

		GPUBuffer visiableInstanceInfos;
		GPUBuffer visiableInstanceCount;

		GPUBuffer offsetBufferCount;

		GPUBuffer outputCommandListsIndirectArgs;
		GPUBuffer shadowDepthIndirectArgs;

		GPUBuffer shadowDepthInstanceOffset;
		GPUBuffer shadowDepthInstanceCounter;

		GPUBuffer drawInstanceInfos;
		GPUBuffer instanceIDs;
		GPUBuffer pageInfos;

		CullingBatchInfo();
	};

	struct ShadowViewInfo
	{
		Matrix4f viewOriginToClip;
		Vector4i viewRect;
		Vector3f worldCenter;
		unsigned int vsmID;
		unsigned int mipLevel;
		unsigned int mipCount;
		unsigned int flags;
		unsigned int pad;
	};
	vector<ShadowViewInfo> shadowViewInfosUpload;
	GPUBuffer shadowViewInfos;

	VirtualShadowMapArray();
	~VirtualShadowMapArray();

	void init(VirtualShadowMapManager& manager);

	bool isAllocated() const;

	VirtualShadowMap* allocate();

	void buildPageAllocations(
		IRenderContext& context,
		const vector<CameraRenderData*>& cameraDatas,
		const LightRenderData& lightData,
		DebugRenderData& debugData
	);

	void render(IRenderContext& context, const LightRenderData& lightData);
	void renderDebugView(IRenderContext& context, const CameraRenderData& mainCameraData);

	void mergeStaticPhysPages(IRenderContext& context);

	void clean();

	Texture2D& getClipmapDebugBuffer();
protected:
	vector<VirtualShadowMap*> cachedShadowMaps;
	vector<VirtualShadowMap*> shadowMaps;
	vector<CullingBatchInfo*> cachedCullingBatchInfos;
	vector<CullingBatchInfo*> cullingBatchInfos;
	Texture2D clipmapDebugBuffer;

	void resizeDirectLightVSMIDs(const vector<CameraRenderData*>& cameraDatas);
	CullingBatchInfo* fetchCullingBatchInfo();

	void buildCullingBatchInfos(const LightRenderData& lightData);

	void cullingPasses(IRenderContext& context, const LightRenderData& lightData);
};