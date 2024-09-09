#pragma once

#include "../RenderCore/SurfaceBufferGetter.h"

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
	ShaderProgram* buildDebugProgram;
	Material* compactMaterial;
	ShaderProgram* compactProgram;
};