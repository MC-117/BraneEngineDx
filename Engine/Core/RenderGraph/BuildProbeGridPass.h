#pragma once

#include "../RenderCore/SurfaceBufferGetter.h"
#include "../PipelineState.h"

class BuildProbeGridPass : public RenderPass
{
public:
	virtual bool loadDefaultResource();
	
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();
protected:
	Material* buildMaterial;
	ShaderProgram* buildProgram;
	ComputePipelineState* buildPSO;
	ShaderProgram* buildDebugProgram;
	ComputePipelineState* buildDebugPSO;
	Material* compactMaterial;
	ShaderProgram* compactProgram;
	ComputePipelineState* compactPSO;
};
