#pragma once

#include "PostProcessPass.h"
#include "../GPUBuffer.h"

class SMAAPass : public PostProcessPass
{
public:
	static Texture2D areaTexture;
	static Texture2D searchTexture;

	Texture2D edgesTexture = Texture2D(size.x, size.y, 2, false, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Linear, TF_Linear_Mip_Linear, TIT_RG8_UF });
	Texture2D blendTexture = Texture2D(size.x, size.y, 4, false, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Linear, TF_Linear_Mip_Linear, TIT_RGBA8_UF });
	Texture2D screenTexture = Texture2D(size.x, size.y, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGB10A2_UF });

	RenderTarget edgesRenderTarget = RenderTarget(size.x, size.y, 2);
	RenderTarget blendRenderTarget = RenderTarget(size.x, size.y, 4);
	RenderTarget screenRenderTarget = RenderTarget(size.x, size.y, 4);

	SMAAPass(const Name& name = "SMAA", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo& info);
	virtual void onGUI(EditorInfo& info);
	virtual void resize(const Unit2Di& size);
protected:
	bool enableDebugEdges = false;
	Material* edgeDetectionMaterial = NULL;
	Material* blendingWeightCalculationMaterial = NULL;
	Material* neighborhoodBlendingMaterial = NULL;
	Material* debugDrawEdgesMaterial = NULL;

	ShaderProgram* edgeDetectionProgram = NULL;
	GraphicsPipelineState* edgeDetectionPSO = NULL;
	ShaderProgram* blendingWeightCalculationProgram = NULL;
	GraphicsPipelineState* blendingWeightCalculationPSO = NULL;
	ShaderProgram* neighborhoodBlendingProgram = NULL;
	GraphicsPipelineState* neighborhoodBlendingPSO = NULL;
	ShaderProgram* debugDrawEdgesProgram = NULL;
	GraphicsPipelineState* debugDrawEdgesPSO = NULL;
};