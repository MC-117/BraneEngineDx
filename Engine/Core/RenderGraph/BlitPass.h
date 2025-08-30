#pragma once
#ifndef _BLITPASS_H_
#define _BLITPASS_H_

#include "../RenderCore/MaterialRenderData.h"

class BlitPass : public RenderPass
{
public:
	virtual bool loadDefaultResource();
	
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	static MaterialRenderData* materialRenderData;
	static IMaterial* materialVariant;
	static GraphicsPipelineState* pipelineState;
};

#endif // !_BLITPASS_H_
