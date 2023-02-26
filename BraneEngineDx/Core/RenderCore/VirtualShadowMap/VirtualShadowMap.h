#pragma once
#include "../RenderInterface.h"
#include "../LightRenderData.h"

struct VirtualShadowMapProjectionData
{
	Matrix4f worldToShadowMat;
	
	Vector2u clipmapOffset;
	int clipmapIndex;
	int clipmapLevel;

	int vsmID;

	unsigned int flag;

	int pad[2];
};

struct VirtualShadowMapPhysicalPageData
{
	unsigned int flag;
	unsigned int age;
};

struct VirtualShadowMapFrameData
{
	GPUBuffer pageTable = GPUBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None);
	GPUBuffer pageFlags = GPUBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None);
	GPUBuffer projectionData = GPUBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None);
	GPUBuffer pageRect = GPUBuffer(GB_Storage, GBF_UInt4, 0, GAF_ReadWrite, CAF_None);
	GPUBuffer physicalPageData = GPUBuffer(GB_Storage, GBF_Struct, sizeof(VirtualShadowMapPhysicalPageData), GAF_ReadWrite, CAF_None);
};

class VirtualShadowMapManager
{
public:
	static constexpr unsigned int pageSize = 128;
	static constexpr unsigned int virtualShadowMapSize = pageSize * pageSize;
	static constexpr unsigned int poolXPageCount = 128;
	static constexpr unsigned int poolWidth = pageSize * poolXPageCount;

	struct ShadowMap
	{
		int preVirtualShadowMapID = -1;
		int curVirtualShadowMapID = -1;

		Vector2u prePageSpaceLocation;
		Vector2u curPageSpaceLocation;

		bool preRendered = false;
		bool curRendered = false;

		bool isValid() const;

		void updateClipMap(MainLightData& lightData);
	};

	struct LightEntry
	{
		int lightID = -1;
		vector<ShadowMap*> shadowMaps;

		ShadowMap* setShadowMap(int index);
	};

	Texture2D poolTexture;
	vector<ShadowMap*> pages;

	VirtualShadowMapManager();

	LightEntry* setLightEntry(int lightID);
	ShadowMap* setShadowMap(int lightID, int index);

	void processInvalidations();
protected:
	unsigned int maxPageCount;
	unordered_map<int, LightEntry*> preShadowMaps;
	unordered_map<int, LightEntry*> curShadowMaps;
};

class VirtualShadowMap
{

};

class VirtualShadowMapArray
{

};