#pragma once

#include "../RenderCore/RenderCommandList.h"

class MeshPass : public RenderPass
{
public:
	RenderCommandList commandList;
	bool requireClearFrame = true;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Timer timer;
	vector<pair<string, Texture*>> outputTextures;
};