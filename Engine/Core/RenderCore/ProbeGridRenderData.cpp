#include "ProbeGridRenderData.h"
#include "CameraRenderData.h"
#include "../Material.h"
#include "../Engine.h"
#include "../Asset.h"
#include "../InitializationManager.h"

ProbeGridConfig& ProbeGridConfig::instance()
{
	static ProbeGridConfig config;
	return config;
}

class ProbeGridConfigInitializer : public Initialization
{
protected:
	static ProbeGridConfigInitializer instance;
	ProbeGridConfigInitializer() : Initialization(
		InitializeStage::BeforeEngineSetup, 0,
		FinalizeStage::BeforeEngineRelease, 0) { }

	virtual bool initialize()
	{
		Engine::engineConfig.configInfo.get("probeGrid.probeGridPixels", (int&)ProbeGridConfig::instance().probeGridPixels);
		Engine::engineConfig.configInfo.get("probeGrid.probeGridZSlices", (int&)ProbeGridConfig::instance().probeGridZSlices);
		Engine::engineConfig.configInfo.get("probeGrid.probeMaxCountPerCell", (int&)ProbeGridConfig::instance().probeMaxCountPerCell);
		Engine::engineConfig.configInfo.get("probeGrid.nearOffsetScale", ProbeGridConfig::instance().nearOffsetScale);
		return true;
	}

	virtual bool finalize()
	{
		return true;
	}
};

ProbeGridConfigInitializer ProbeGridConfigInitializer::instance;

Vector3f GetLightGridZParams(float NearPlane, float FarPlane, unsigned int lightGridSizeZ, float nearOffsetScale)
{
	// S = distribution scale
	// B, O are solved for given the z distances of the first+last slice, and the # of slices.
	//
	// slice = log2(z*B + O) * S

	// Don't spend lots of resolution right in front of the near plane
	double NearOffset = .095 * nearOffsetScale;
	// Space out the slices so they aren't all clustered at the near plane
	double S = 4.05;

	double N = NearPlane + NearOffset;
	double F = FarPlane;

	double O = (F - N * exp2((lightGridSizeZ - 1) / S)) / (F - N);
	double B = (1 - O) / N;

	return Vector3f(B, O, S);
}

ProbeGridInfo::ProbeGridInfo()
	: debugProbeIndex(-1)
{
}

void ProbeGridInfo::init(int width, int height, float zNear, float zFar, const ProbePoolRenderData& probePool)
{
	ProbeGridConfig& config = ProbeGridConfig::instance();
	probeGridSize.x() = ceil(width / (float)config.probeGridPixels);
	probeGridSize.y() = ceil(height / (float)config.probeGridPixels);
	probeGridSize.z() = config.probeGridZSlices;
	probeGridPixelSizeShift = floor(log2((float)config.probeGridPixels));
	probeGridZParams = GetLightGridZParams(zNear, zFar + 10.0f, config.probeGridZSlices, config.nearOffsetScale);
	probeByteWidth = ProbeByteSize;
	probeCount = probePool.getProbeCount();
	probeGridCellCount = probeGridSize.x() * probeGridSize.y() * probeGridSize.z();
	probeMaxCountPerCell = config.probeMaxCountPerCell;
	probeMaxLinkListLength = probeGridCellCount * probeMaxCountPerCell * ProbeType::ProbeType_Count;
}

ProbeByteData& ProbePoolRenderData::emplace(ProbeType type, int& outProbeIndex)
{
	outProbeIndex = byteData.size();
	ProbeByteData& data = byteData.emplace_back();
	data.probeType = type;
	return data;
}

ProbeByteData& ProbePoolRenderData::getProbeByteData(int probeIndex)
{
	return byteData[probeIndex];
}

const ProbeByteData& ProbePoolRenderData::getProbeByteData(int probeIndex) const
{
	return byteData[probeIndex];
}

int ProbePoolRenderData::getProbeCount() const
{
	return byteData.size();
}

void ProbePoolRenderData::create()
{
}

void ProbePoolRenderData::release()
{
	byteDataBuffer.resize(0);
}

void ProbePoolRenderData::upload()
{
	if (byteData.empty()) {
		byteDataBuffer.resize(0);
	}
	byteDataBuffer.uploadData(byteData.size(), byteData.data());
}

void ProbePoolRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName probeByteDataName = "probeByteData";
	context.bindBufferBase(byteDataBuffer.getVendorGPUBuffer(), probeByteDataName);
}

void ProbePoolRenderData::clean()
{
	byteData.clear();
}

Material* ProbeGridRenderData::buildMaterial = NULL;
ShaderProgram* ProbeGridRenderData::buildProgram = NULL;
Material* ProbeGridRenderData::compactMaterial = NULL;
ShaderProgram* ProbeGridRenderData::compactProgram = NULL;
bool ProbeGridRenderData::isInited = false;

const ShaderPropertyName ProbeGridRenderData::outProbeGridLinkListName = "outProbeGridLinkList";
const ShaderPropertyName ProbeGridRenderData::outProbeGridLinkHeadName = "outProbeGridLinkHead";
const ShaderPropertyName ProbeGridRenderData::probeGridLinkListName = "probeGridLinkList";
const ShaderPropertyName ProbeGridRenderData::probeGridLinkHeadName = "probeGridLinkHead";

const ShaderPropertyName ProbeGridRenderData::outProbeGridCulledListName = "outProbeGridCulledList";
const ShaderPropertyName ProbeGridRenderData::outProbeGridCulledHeadName = "outProbeGridCulledHead";
const ShaderPropertyName ProbeGridRenderData::probeGridCulledListName = "probeGridCulledList";
const ShaderPropertyName ProbeGridRenderData::probeGridCulledHeadName = "probeGridCulledHead";

void ProbeGridRenderData::create()
{
}

void ProbeGridRenderData::release()
{
}

void ProbeGridRenderData::upload()
{
}

void ProbeGridRenderData::bind(IRenderContext& context)
{
	context.bindBufferBase(probeGridCulledListBuffer.getVendorGPUBuffer(), probeGridCulledListName);
	context.bindBufferBase(probeGridCulledHeadBuffer.getVendorGPUBuffer(), probeGridCulledHeadName);
}

void ProbeGridRenderData::clean()
{
}
