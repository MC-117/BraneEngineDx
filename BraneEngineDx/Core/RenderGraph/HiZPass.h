#pragma once

#include "../RenderCore/RenderInterface.h"
#include "../RenderTarget.h"

class HiZPass : public RenderPass
{
public:
	Texture* depthTexture = NULL;
	Texture* hizTexture = NULL;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Material* material;
	ShaderProgram* program;
};