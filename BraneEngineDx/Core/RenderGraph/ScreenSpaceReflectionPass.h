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
	Texture* hiZMap = NULL;

	Texture* hitDataMap = NULL;
	Texture* hitColorMap = NULL;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Timer timer;

	Material* material = NULL;
	ShaderProgram* program = NULL;
	MaterialRenderData materialRenderData;
};