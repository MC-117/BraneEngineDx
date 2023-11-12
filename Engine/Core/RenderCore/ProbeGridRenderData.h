#pragma once

#include "RenderInterface.h"
#include "ProbeTypes.h"
#include "../GPUBuffer.h"

struct ProbeGridConfig
{
	unsigned int probeGridPixels = 64;
	unsigned int probeGridZSlices = 64;
	unsigned int probeMaxCountPerCell = 16;
	float nearOffsetScale = 10;

	static ProbeGridConfig& instance();
};

class ProbePoolRenderData;

struct ProbeGridInfo
{
	Vector3u probeGridSize;
	unsigned int probeGridPixelSizeShift;
	Vector3f probeGridZParams;
	unsigned int probeByteWidth;
	unsigned int probeCount;
	unsigned int probeGridCellCount;
	unsigned int probeMaxCountPerCell;
	unsigned int probeMaxLinkListLength;
	int debugProbeIndex;
	int pad[3];

	ProbeGridInfo();

	void init(int width, int height, float zNear, float zFar, const ProbePoolRenderData& probePool);
};

class ProbePoolRenderData : public IRenderData
{
public:
	vector<ProbeByteData> byteData;
	GPUBuffer byteDataBuffer = GPUBuffer(GB_Storage, GBF_Raw, ProbeByteSize);

	ProbeByteData& emplace(ProbeType type, int& outProbeIndex);
	ProbeByteData& getProbeByteData(int probeIndex);
	const ProbeByteData& getProbeByteData(int probeIndex) const;
	int getProbeCount() const;

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};

class CameraRenderData;

class ProbeGridRenderData : public IRenderData
{
public:
	static const ShaderPropertyName outProbeGridLinkListName;
	static const ShaderPropertyName outProbeGridLinkHeadName;
	static const ShaderPropertyName probeGridLinkListName;
	static const ShaderPropertyName probeGridLinkHeadName;

	static const ShaderPropertyName outProbeGridCulledListName;
	static const ShaderPropertyName outProbeGridCulledHeadName;
	static const ShaderPropertyName probeGridCulledListName;
	static const ShaderPropertyName probeGridCulledHeadName;

	ProbePoolRenderData* probePool = NULL;

	GPUBuffer probeGridCulledListBuffer = GPUBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None);
	GPUBuffer probeGridCulledHeadBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(int) * ProbeType::ProbeType_Range, GAF_ReadWrite, CAF_None);

	GPUBuffer probeGridLinkListBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(int) * 2, GAF_ReadWrite, CAF_None);
	GPUBuffer probeGridLinkHeadBuffer = GPUBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None);

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
protected:
	static Material* buildMaterial;
	static ShaderProgram* buildProgram;
	static Material* compactMaterial;
	static ShaderProgram* compactProgram;
	static bool isInited;

	static void loadDefaultResource();
};
