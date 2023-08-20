#include "EnvLightData.h"
#include "../ProbeSystem/EnvLightCaptureProbeRender.h"
#include "../Asset.h"
#include "../Profile/ProfileCore.h"

void EnvLightSHData::set(const SHCoeff3RGB& sh)
{
	const float sqrtPI = sqrt(PI);
	const float coeff0 = 1.0f / (2 * sqrtPI);
	const float coeff1 = sqrt(3.f) / (3 * sqrtPI);
	const float coeff2 = sqrt(15.f) / (8 * sqrtPI);
	const float coeff3 = sqrt(5.f) / (16 * sqrtPI);
	const float coeff4 = .5f * coeff2;

	v[0].x() = -coeff1 * sh.r.d[3];
	v[0].y() = -coeff1 * sh.r.d[1];
	v[0].z() = coeff1 * sh.r.d[2];
	v[0].w() = coeff0 * sh.r.d[0] - coeff3 * sh.r.d[6];

	v[1].x() = -coeff1 * sh.g.d[3];
	v[1].y() = -coeff1 * sh.g.d[1];
	v[1].z() = coeff1 * sh.g.d[2];
	v[1].w() = coeff0 * sh.g.d[0] - coeff3 * sh.g.d[6];

	v[2].x() = -coeff1 * sh.b.d[3];
	v[2].y() = -coeff1 * sh.b.d[1];
	v[2].z() = coeff1 * sh.b.d[2];
	v[2].w() = coeff0 * sh.b.d[0] - coeff3 * sh.b.d[6];

	v[3].x() = coeff2 * sh.r.d[4];
	v[3].y() = -coeff2 * sh.r.d[5];
	v[3].z() = 3 * coeff3 * sh.r.d[6];
	v[3].w() = -coeff2 * sh.r.d[7];

	v[4].x() = coeff2 * sh.g.d[4];
	v[4].y() = -coeff2 * sh.g.d[5];
	v[4].z() = 3 * coeff3 * sh.g.d[6];
	v[4].w() = -coeff2 * sh.g.d[7];

	v[5].x() = coeff2 * sh.b.d[4];
	v[5].y() = -coeff2 * sh.b.d[5];
	v[5].z() = 3 * coeff3 * sh.b.d[6];
	v[5].w() = -coeff2 * sh.b.d[7];

	v[6].x() = coeff4 * sh.r.d[8];
	v[6].y() = coeff4 * sh.g.d[8];
	v[6].z() = coeff4 * sh.b.d[8];
	v[6].w() = 1;
}

RGBASHCoeff3Data& RGBASHCoeff3Data::operator+=(const RGBASHCoeff3Data& v)
{
	for (int i = 0; i < SHCoeff3::CoeffCount; i++)
		coeffs[i] = v.coeffs[i];
	return *this;
}

Material* EnvLightProbeRenderData::material = NULL;
ShaderProgram* EnvLightProbeRenderData::program = NULL;
bool EnvLightProbeRenderData::isInited = false;

int EnvLightProbeRenderData::setLightCapture(EnvLightCaptureProbeRender* capture)
{
	captures.push_back(capture);
	return captures.size() - 1;
}

void EnvLightProbeRenderData::create()
{
	envSHDataBuffer.resize(CF_Faces);
	readBackBuffer.resize(CF_Faces);
	loadDefaultResource();
	program->init();
}

void EnvLightProbeRenderData::release()
{
	envSHDataBuffer.resize(0);
	readBackBuffer.resize(0);
}

void EnvLightProbeRenderData::upload()
{
}

void EnvLightProbeRenderData::bind(IRenderContext& context)
{
}

void EnvLightProbeRenderData::clean()
{
	captures.clear();
}

void EnvLightProbeRenderData::computeEnvLight(IRenderContext& context, EnvLightCaptureProbeRender& capture)
{
	TextureCube* cubeMap = capture.getLightCubeMap();
	if (cubeMap == NULL)
		return;

	static const ShaderPropertyName lightCubeMapName = "lightCubeMap";
	static const ShaderPropertyName outSHCoeffsName = "outSHCoeffs";

	context.bindShaderProgram(program);
	MipOption mipOption;
	mipOption.dimension = TD_Array;
	mipOption.arrayCount = CF_Faces;
	context.bindTexture((ITexture*)cubeMap->getVendorTexture(), lightCubeMapName, mipOption);
	context.bindBufferBase(envSHDataBuffer.getVendorGPUBuffer(), outSHCoeffsName, { true });
	context.dispatchCompute(1, 1, CF_Faces);
	context.bindTexture(NULL, lightCubeMapName);
	context.clearOutputBufferBindings();

	RGBASHCoeff3Data envSHdata[CF_Faces];
	context.copyBufferData(envSHDataBuffer.getVendorGPUBuffer(), readBackBuffer.getVendorGPUBuffer());
	readBackBuffer.readData(envSHdata);

	SHCoeff3RGB& shCoeff3RGB = capture.shCoeff3RGB;
	for (int c = 0; c < SHCoeff3::CoeffCount; c++) {
		Vector4f rgbaCoeff = Vector4f(0, 0, 0, 0);
		for (int f = 0; f < CF_Faces; f++) {
			rgbaCoeff += envSHdata[f].coeffs[c];
		}
		Vector3f rgbCoeff = rgbaCoeff.block(0, 0, 3, 1);
		rgbCoeff *= 4 * PI / max(rgbaCoeff.w(), 0.00001f);
		shCoeff3RGB.r.d[c] = rgbCoeff.x();
		shCoeff3RGB.g.d[c] = rgbCoeff.y();
		shCoeff3RGB.b.d[c] = rgbCoeff.z();
	}
}

void EnvLightProbeRenderData::computeEnvLights(IRenderContext& context)
{
	for (EnvLightCaptureProbeRender* capture : captures)
	{
		if (capture == NULL)
			continue;
		computeEnvLight(context, *capture);
	}
}

void EnvLightProbeRenderData::loadDefaultResource()
{
	if (isInited)
		return;
	material = getAssetByPath<Material>("Engine/Shaders/Pipeline/CalEnvLighting.mat");
	if (material == NULL)
		throw runtime_error("Not found default shader");
	program = material->getShader()->getProgram(Shader_Default);
	if (program == NULL)
		throw runtime_error("Not found default shader");
	isInited = true;
}

EnvLightRenderData::EnvLightRenderData(ProbePoolRenderData& probePool) : probePool(probePool)
{
}

int EnvLightRenderData::setLightData(EnvLightCaptureProbeRender* capture)
{
	int probeIndex = -1;
	EnvLightData& data = probePool.emplace(ProbeType::ProbeType_Env, probeIndex).envLightData;
	data.tintColor = Vector3f(capture->tintColor.r, capture->tintColor.g, capture->tintColor.b);
	data.reverseIndex = 0;
	data.position = capture->getWorldPosition();
	data.radius = capture->getRadius();
	data.cutoff = capture->cutoff;
	data.falloff = capture->falloff;
	data.shDataOffset = envLightSHData.size();
	probeIndices.push_back(probeIndex);
	EnvLightSHData& shData = envLightSHData.emplace_back();
	shData.set(capture->shCoeff3RGB);
	return probeIndex;
}

void EnvLightRenderData::create()
{
}

void EnvLightRenderData::release()
{
	envLightSHDataBuffer.resize(0);
}

void EnvLightRenderData::upload()
{
	envLightSHDataBuffer.uploadData(envLightSHData.size() * EnvLightSHData::SHFLoat4Count, envLightSHData.data());
}

void EnvLightRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName envLightSHDatasName = "envLightSHDatas";
	context.bindBufferBase(envLightSHDataBuffer.getVendorGPUBuffer(), envLightSHDatasName);
}

void EnvLightRenderData::clean()
{
	probeIndices.clear();
	envLightSHData.clear();
}
