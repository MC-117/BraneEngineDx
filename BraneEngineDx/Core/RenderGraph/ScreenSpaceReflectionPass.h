#pragma once

#include "../RenderCore/RenderCommandList.h"
#include "../RenderCore/SurfaceBufferGetter.h"

class ScreenSpaceReflectionPass : public RenderPass
{
public:
	bool enable = true;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	vector<pair<string, Texture*>> outputTextures;
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