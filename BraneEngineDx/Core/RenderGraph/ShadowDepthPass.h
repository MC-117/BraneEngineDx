#pragma once

#include "../RenderCore/RenderCommandList.h"

class ShadowDepthPass : public RenderPass
{
public:
	RenderCommandList commandList;
	bool requireClearFrame = true;

	virtual bool setRenderCommand(const IRenderCommand& cmd);
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Material* material;
	Timer timer;
	vector<pair<string, Texture*>> outputTextures;
};