#pragma once

#include "../RenderCore/RenderInterface.h"

class ImGuiPass : public RenderPass
{
public:
	ImDrawData* drawData;

	virtual bool loadDefaultResource();

	void setDrawData(ImDrawData* pDrawData);

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();
};