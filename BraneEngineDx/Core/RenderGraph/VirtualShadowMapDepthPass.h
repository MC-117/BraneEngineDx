#pragma once

#include "MeshPass.h"

class VirtualShadowMapDepthPass: public RenderPass
{
public:
	virtual bool setRenderCommand(const IRenderCommand& cmd);

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Timer timer;
	vector<pair<string, Texture*>> outputTextures;
};