#pragma once

#include "ProbeGridRenderData.h"
#include "../Utility/SHMath.h"
#include "../RenderTarget.h"
#include "../TextureCube.h"

class EnvLightCaptureProbeRender;

struct EnvLightSHData
{
	static constexpr int SHFLoat4Count = 7;
	Vector4f v[SHFLoat4Count];

	void set(const SHCoeff3RGB& sh);
};

struct RGBASHCoeff3Data
{
	Vector4f coeffs[SHCoeff3::CoeffCount];

	RGBASHCoeff3Data& operator+=(const RGBASHCoeff3Data& v);
};

struct EnvLightProbeData
{
	SHCoeff3RGB* shCoeff3RGB = NULL;
	TextureCube* lightCubeMap = NULL;
};

struct EnvLightProbeRenderData : public IRenderData
{
	vector<EnvLightProbeData> datas;
	GPUBuffer envSHDataBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(RGBASHCoeff3Data), GAF_ReadWrite, CAF_None);
	GPUBuffer readBackBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(RGBASHCoeff3Data), GAF_ReadWrite, CAF_Read);

	int setLightCapture(const EnvLightProbeData& data);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();

	void computeEnvLight(IRenderContext& context, EnvLightProbeData& data);
	void computeEnvLights(IRenderContext& context);
protected:
	static Material* material;
	static ShaderProgram* program;
	static bool isInited;

	static void loadDefaultResource();
};

struct EnvLightUpdateData
{
	Vector3f position;
	float radius = 0;
	Vector3f tintColor;
	float falloff = 0;
	float cutoff = 0;
	SHCoeff3RGB shCoeff3RGB;
};

struct EnvLightRenderData : public IRenderData
{
	ProbePoolRenderData& probePool;
	vector<int> probeIndices;
	vector<EnvLightSHData> envLightSHData;
	GPUBuffer envLightSHDataBuffer = GPUBuffer(GB_Storage, GBF_Float4);

	EnvLightRenderData(ProbePoolRenderData& probePool);

	int setLightData(const EnvLightUpdateData& data);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};