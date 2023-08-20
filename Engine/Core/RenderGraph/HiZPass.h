#pragma once

#include "../RenderCore/SurfaceBufferGetter.h"

class HiZPass : public RenderPass
{
public:
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	vector<pair<string, Texture*>> outputTextures;
	Material* material;
	ShaderProgram* program;
};