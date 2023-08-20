#pragma once

#include "../RenderCore/SurfaceBufferGetter.h"
#include "../RenderCore/VirtualShadowMap/VirtualShadowMapRenderData.h"

class VirtualShadowMapDepthPass: public RenderPass
{
public:
	virtual bool setRenderCommand(const IRenderCommand& cmd);

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	static Material* vsmMaterial;
	static void loadDefaultResource();
	Timer timer;
	vector<pair<string, Texture*>> outputTextures;
};