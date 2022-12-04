#pragma once

#include "../RenderCore/RenderCommandList.h"
#include "../RenderTarget.h"

class ScreenSpaceReflectionPass : public RenderPass
{
public:
	bool enable = true;
	SceneRenderData* sceneData = NULL;
	CameraRenderData* cameraData = NULL;

	Texture* gBufferA = NULL;
	Texture* gBufferB = NULL;
	Texture* gBufferC = NULL;
	Texture* gBufferE = NULL;
	Texture* hiZMap = NULL;

	Texture* hitDataMap = NULL;
	Texture* hitColorMap = NULL;
	RenderTarget* traceRenderTarget = NULL;
	RenderTarget* resolveRenderTarget = NULL;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Timer timer;

	bool useComputeShader = false;

	Material* computeMaterial = NULL;
	ShaderProgram* computeProgram = NULL;
	MaterialRenderData computeMaterialRenderData;

	Material* traceMaterial = NULL;
	ShaderProgram* traceProgram = NULL;
	Material* resolveMaterial = NULL;
	ShaderProgram* resolveProgram = NULL;
	MaterialRenderData traceMaterialRenderData;
	MaterialRenderData resolveMaterialRenderData;
};