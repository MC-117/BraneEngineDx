#pragma once

#include "../RenderCore/RenderCommandList.h"

class ReflectionProbeArrayPass : public RenderPass
{
public:

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Timer timer;
	vector<pair<string, Texture*>> outputTextures;

	
};