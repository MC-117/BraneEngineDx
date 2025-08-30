#pragma once

#include "../RenderCore/SurfaceBufferGetter.h"
#include "../PipelineState.h"

class GenMipPass : public RenderPass
{
public:
	virtual bool loadDefaultResource();
	
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Material* material;
	ShaderProgram* program;
	ComputePipelineState* pipelineState;

	vector<pair<string, Texture*>> outputTextures;
};
