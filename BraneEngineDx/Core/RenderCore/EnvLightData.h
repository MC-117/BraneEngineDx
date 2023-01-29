#pragma once

#include "RenderInterface.h"
#include "../Utility/SHMath.h"
#include "../GPUBuffer.h"
#include "../RenderTarget.h"
#include "../TextureCube.h"

class EnvLightCaptureProbeRender;

struct EnvLightData
{
	Vector3f tintColor;
	int reverseIndex = 0;
	Vector3f position;
	float radius = 0;
	float falloff = 0;
	float cutoff = 0;
	float pad[2];
	Vector4f v[7];

	void set(const SHCoeff3RGB& sh);
};

struct RGBASHCoeff3Data
{
	Vector4f coeffs[SHCoeff3::CoeffCount];

	RGBASHCoeff3Data& operator+=(const RGBASHCoeff3Data& v);
};

struct EnvLightProbeRenderData : public IRenderData
{
	vector<EnvLightCaptureProbeRender*> captures;
	GPUBuffer envSHDataBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(RGBASHCoeff3Data), GAF_ReadWrite, CAF_None);
	GPUBuffer readBackBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(RGBASHCoeff3Data), GAF_ReadWrite, CAF_Read);

	int setLightCapture(EnvLightCaptureProbeRender* capture);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();

	void computeEnvLight(IRenderContext& context, EnvLightCaptureProbeRender& capture);
	void computeEnvLights(IRenderContext& context);
protected:
	static Material* material;
	static ShaderProgram* program;
	static bool isInited;

	static void loadDefaultResource();
};

struct EnvLightRenderData : public IRenderData
{
	vector<EnvLightData> envLightdata;
	GPUBuffer envLightDataBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(EnvLightData));

	int setLightData(EnvLightCaptureProbeRender* capture);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};