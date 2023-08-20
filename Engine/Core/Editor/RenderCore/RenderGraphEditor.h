#pragma once

#include "RenderPassEditor.h"

class RenderGraphEditor : public BaseEditor
{
public:
	RenderGraphEditor() = default;
	virtual ~RenderGraphEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onRenderGraphGUI(EditorInfo & info);

	virtual void onGUI(EditorInfo & info);
protected:
	RenderGraph* renderGraph = NULL;
};