#pragma once

#include "../RenderCore/RenderCommandList.h"

class MeshPass : public RenderPass
{
public:
	RenderCommandList commandList;
	Enum<ClearFlags> plusClearFlags = Clear_None;
	Enum<ClearFlags> minusClearFlags = Clear_None;

	virtual bool loadDefaultResource();

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Timer timer;
	vector<pair<string, Texture*>> outputTextures;
};