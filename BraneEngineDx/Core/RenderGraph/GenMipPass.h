#pragma once

#include "../RenderCore/SurfaceBufferGetter.h"

class GenMipPass : public RenderPass
{
public:
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Material* material;
	ShaderProgram* program;

	vector<pair<string, Texture*>> outputTextures;
};