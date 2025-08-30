#pragma once

#include "../RenderCore/RenderCommandList.h"
#include "../RenderCore/SurfaceBufferGetter.h"
#include "SSRBindings.h"

class ScreenSpaceReflectionPass : public RenderPass
{
public:
	bool enable = true;
	SSRBinding ssrBinding;

	virtual bool loadDefaultResource();

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	vector<pair<string, Texture*>> outputTextures;
	Timer timer;

	bool useComputeShader = false;

	MaterialRenderData* computeMaterialRenderData = NULL;
	IMaterial* computeMaterialVariant = NULL;
	ComputePipelineState* computePSO = NULL;

	Material* traceMaterial = NULL;
	ShaderProgram* traceProgram = NULL;
	GraphicsPipelineState* tracePSO = NULL;
	Material* resolveMaterial = NULL;
	ShaderProgram* resolveProgram = NULL;
	GraphicsPipelineState* resolvePSO = NULL;
};