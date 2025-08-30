#pragma once
#include "PostProcessPass.h"
#include "../GPUBuffer.h"

struct FogParameters
{
	float efogDensity = 0.02 * 0.001;
	float efogHeight = 0;
	float efogHeightFalloff = 0.2 * 0.001;
	float efogStartDistance = 0;
	Vector3f efogColor;
	float efogCutoffDistance = 0;
	float efogMaxOpacity = 1;
	float vfogScatteringDistribution = 0.2;
	float vfogExtinctionScale = 1;
	float vfogStartDistance = 0;
	Vector3f vfogColor;
	float vfogMaxDistance = 300;
	Vector3f fogGridZParams;
	float fogMaxWorldViewHeight = 0;
	Vector3i fogGridSize;
	float vfogNoiseScalar = 0.5f;
	Vector2f fogScreenToUV;
	Vector2f hiZSize;
};

struct VolumetricFogConfig
{
	unsigned int gridPixels = 16;
	unsigned int gridZSlices = 128;
	float nearOffsetScale = 0.1;
	float depthDistributionScale = 10.0f;

	static VolumetricFogConfig& instance();
};

class VolumetricLightPass : public PostProcessPass
{
public:
	Texture2D maxDepthTexture;
	Texture3D fogScatteringVolume;
	Texture3D fogIntegratedVolume;

	FogParameters parameters;
	
	VolumetricLightPass(const Name& name = "VolumetricLight", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
	virtual void onGUI(EditorInfo& info);
protected:
	int safeWidth = 0, safeHeight = 0, safeDepth = 0;
	Texture* directShadowMap = NULL;
	Texture* hiZTexture = NULL;
	IRenderData* vsmRenderData = NULL;
	IRenderData* lightRenderData = NULL;
	IRenderData* probeRenderData = NULL;
	GPUBuffer parameterBuffer;
	Material* genMaxDepthMaterial = NULL;
	ShaderProgram* genMaxDepthProgram = NULL;
	ComputePipelineState* genMaxDepthPipelineState = NULL;
	Material* lightScatteringMaterial = NULL;
	ShaderProgram* lightScatteringProgram = NULL;
	ComputePipelineState* lightScatteringPipelineState = NULL;
	Material* integrationMaterial = NULL;
	ShaderProgram* integrationProgram = NULL;
	ComputePipelineState* integrationPipelineState = NULL;
	Material* applyMaterial = NULL;
	ShaderProgram* applyProgram = NULL;
	GraphicsPipelineState* applyPipelineState = NULL;
};

