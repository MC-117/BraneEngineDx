#pragma once
#ifndef _BLITPASS_H_
#define _BLITPASS_H_

#include "../RenderCore/MaterialRenderData.h"

class BlitPass : public RenderPass
{
public:
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	static Material* material;
	static ShaderProgram* program;
	static bool isInited;
	MaterialRenderData* materialRenderData = NULL;

	static void loadDefaultResource();
};

#endif // !_BLITPASS_H_
