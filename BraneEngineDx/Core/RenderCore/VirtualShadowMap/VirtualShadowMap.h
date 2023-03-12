#pragma once
#include "../RenderInterface.h"
#include "../LightRenderData.h"
#include "../TransformRenderData.h"

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

	unsigned int physPoolWidth = 128 * 128;
	unsigned int maxPhysPages = 4096;
	unsigned int firstClipmapLevel = 6;
	unsigned int lastClipmapLevel = 16;

	float clipmapRadiusZScale = 1000.0f;
	float resolutionLodBiasClipmap = -0.5f;

	float pageDilationBorderSizeMain = 0.05f;
	float pageDilationBorderSizeLocal = 0.05f;

	Vector2u getPhysPagesXY() const;

	static VirtualShadowMapConfig& instance();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

struct VirtualShadowMapProjectionData
{
	Matrix4f worldToView;
	Matrix4f viewToClip;
	Matrix4f worldToClip;
	Matrix4f worldToUV;

	Vector3f clipmapWorldOrigin;
	int resolutionLodBias;
	
	Vector2i clipmapOffset;
	int clipmapIndex;
	int clipmapLevel;

	int clipmapCount;

	int vsmID;

	int lightType;

	int uncached;
};

struct VirtualShadowMapInfo
{
	unsigned int physPageRowMask;
	unsigned int physPageRowShift;
	unsigned int maxPhysPages;
	unsigned int numShadowMapSlots;
	Vector2u physPoolSize;
	Vector2u physPoolPages;
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
	GPUBuffer* vsmInfo = NULL;
	GPUBuffer* pageTable = NULL;
	GPUBuffer* pageFlags = NULL;
	GPUBuffer* projData = NULL;
	GPUBuffer* pageRect = NULL;
	GPUBuffer* physPageMetaData = NULL;

	~VirtualShadowMapFrameData();

	void create();
	void release();
	void swap(VirtualShadowMapFrameData& data);
};

class VirtualShadowMapShaders
{
public:
	static ShaderProgram* procInvalidationsProgram;
	static ShaderProgram* initPageRectsProgram;
	static ShaderProgram* generatePageFlagsFromPixelsProgram;
	static ShaderProgram* initPhysicalPageMetaDataProgram;
	static ShaderProgram* createCachedPageMappingsProgram;
	static ShaderProgram* packFreePagesProgram;
	static ShaderProgram* allocateNewPageMappingsProgram;
	static ShaderProgram* generateHierarchicalPageFlagsProgram;
	static ShaderProgram* clearInitPhysPagesIndirectArgsProgram;
	static ShaderProgram* selectPagesToInitProgram;
	static ShaderProgram* initPhysicalMemoryIndirectProgram;
	static ShaderProgram* cullPerPageDrawCommandsProgram;
	static ShaderProgram* allocateCommandInstanceOutputSpaceProgram;
	static ShaderProgram* initOutputommandInstanceListsArgsProgram;
	static ShaderProgram* outputCommandInstanceListsProgram;

	static Vector3u procInvalidationsProgramDim;
	static Vector3u initPageRectsProgramDim;
	static Vector3u generatePageFlagsFromPixelsProgramDim;
	static Vector3u initPhysicalPageMetaDataProgramDim;
	static Vector3u createCachedPageMappingsProgramDim;
	static Vector3u packFreePagesProgramDim;
	static Vector3u allocateNewPageMappingsProgramDim;
	static Vector3u generateHierarchicalPageFlagsProgramDim;
	static Vector3u clearInitPhysPagesIndirectArgsProgramDim;
	static Vector3u selectPagesToInitProgramDim;
	static Vector3u initPhysicalMemoryIndirectProgramDim;
	static Vector3u cullPerPageDrawCommandsProgramDim;
	static Vector3u allocateCommandInstanceOutputSpaceProgramDim;
	static Vector3u initOutputommandInstanceListsArgsProgramDim;
	static Vector3u outputCommandInstanceListsProgramDim;

	static const ShaderPropertyName VSMBuffInfoName;
	static const ShaderPropertyName vsmPrevDataName;
	static const ShaderPropertyName pageTableName;
	static const ShaderPropertyName pageFlagsName;
	static const ShaderPropertyName pageRectName;
	static const ShaderPropertyName physPageMetaDataName;
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

	static const ShaderPropertyName outDrawInstanceInfosName;
	static const ShaderPropertyName outVisiableInstanceInfosName;
	static const ShaderPropertyName outVisiableInstanceCountName;
	static const ShaderPropertyName outOffsetBufferCountName;
	static const ShaderPropertyName outShadowDepthInstanceOffsetName;

	static const ShaderPropertyName outputCommandListsIndirectArgsName;
	static const ShaderPropertyName outShadowDepthIndirectArgsName;

	static const ShaderPropertyName pageInfoName;
	static const ShaderPropertyName outPageInfoName;

	static void loadDefaultResource();
};

class VirtualShadowMap;

struct VSMDrawInstanceInfo
{
	unsigned int instanceID;
};

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

		bool preRendered = false;
		bool curRendered = false;

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
	};

	struct LightEntry
	{
		int lightID = -1;
		int preRenderFrame = -1;
		int curRenderFrame = -1;
		vector<ShadowMap*> shadowMaps;
		vector<VSMDrawInstanceInfo> drawInstanceInfos;
		vector<DrawElementsIndirectCommand> indirectCommands;

		ShadowMap* setShadowMap(int index);

		void updateClipmap();
		void addMeshCommand(const DirectShadowRenderCommand& command);
		void markRendered(unsigned int frame);

		void invalidate();
	};

	struct InvalidationInfo
	{
		int numUpdateInstances;
		int pad[3];
	};
	GPUBuffer invalidationInfoBuffer;

	VirtualShadowMapFrameData prevFrameData;
	VirtualShadowMapInfo prevShadowMapInfo;

	Texture2D poolTexture;
	vector<ShadowMap*> pages;

	VirtualShadowMapManager();

	void invalidate();
	void setPoolTextureSize(Vector2u size);
	LightEntry* setLightEntry(int lightID, int cameraID);

	void prepare();

	void buildPrevData(const vector<VirtualShadowMap*>& shadowMaps, vector<VirtualShadowMapPrevData>& prevData);
	void bindPrevFrameData(IRenderContext& context);

	void processInvalidations(IRenderContext& context, MeshTransformRenderData& transformData);
protected:
	unsigned int maxPageCount;
	unordered_map<unsigned long long, LightEntry*> preShadowMaps;
	unordered_map<unsigned long long, LightEntry*> curShadowMaps;
};

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

	VirtualShadowMapInfo vsmInfoUpload;

	GPUBuffer vsmPrevData;
	GPUBuffer vsmInfo;
	GPUBuffer projData;
	GPUBuffer pageRect;
	GPUBuffer pageRequestFlags;
	vector<GPUBuffer*> directLightVSMIDs;
	GPUBuffer pageTable;
	GPUBuffer pageFlags;
	GPUBuffer freePhysPages;
	GPUBuffer physPageAllocRequests;
	GPUBuffer physPageMetaData;
	GPUBuffer allocPageRect;

	GPUBuffer initPhysPagesIndirectArgs;
	GPUBuffer physPagesToInit;

	struct CullingData
	{
		unsigned int instanceCount;
		unsigned int firstView;
		unsigned int viewCount;
		unsigned int maxPerInstanceCmdCount;
		unsigned int frame;
	};

	struct VisiableInstanceInfo
	{
		unsigned int viewIndex;
		VSMDrawInstanceInfo drawInfo;
		unsigned int indirectArgIndex;
	};

	struct AllocCmdInfo
	{
		unsigned int indirectArgCount;
	};

	struct CullingBatchInfo
	{
		CullingData data;
		VirtualShadowMapManager::LightEntry* lightEntry;
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
		GPUBuffer pageInfo;

		CullingBatchInfo();
	};

	struct ShadowViewInfo
	{
		Vector4i viewRect;
		unsigned int vsmID;
		unsigned int mipLevel;
		unsigned int mipCount;
		unsigned int flags;
	};
	vector<ShadowViewInfo> shadowViewInfosUpload;
	GPUBuffer shadowViewInfos;

	VirtualShadowMapArray();
	~VirtualShadowMapArray();

	void init(IRenderContext& context, VirtualShadowMapManager* manager);

	VirtualShadowMap* allocate();

	void buildPageAllocations(
		IRenderContext& context,
		Texture2D& sceneDepthMap,
		Texture2D& sceneNormalMap,
		const vector<CameraRenderData*>& cameraDatas,
		const LightRenderData& lightData
	);

	void clean();
protected:
	vector<VirtualShadowMap*> shadowMaps;
	vector<CullingBatchInfo*> cachedCullingBatchInfos;
	vector<CullingBatchInfo*> cullingBatchInfos;

	void resizeDirectLightVSMIDs(const vector<CameraRenderData*>& cameraDatas);
	CullingBatchInfo* fetchCullingBatchInfo();

	void buildCullingBatchInfos(const LightRenderData& lightData);

	void cullingPasses(IRenderContext& context, const LightRenderData& lightData);
};