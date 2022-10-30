#pragma once

#include "../RenderCore/RenderInterface.h"
#include "../RenderCore/MaterialRenderData.h"
#include "../RenderCore/CameraRenderData.h"
#include "../RenderTarget.h"

class ScreenSpaceReflectionPass : RenderPass
{
public:
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
	MaterialRenderData* materialRenderData = NULL;
};