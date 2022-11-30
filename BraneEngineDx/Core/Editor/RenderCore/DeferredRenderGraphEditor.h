#pragma once

#include "RenderGraphEditor.h"
#include "../../RenderGraph/DeferredRenderGraph.h"

class DeferredRenderGraphEditor : public RenderGraphEditor
{
public:
	DeferredRenderGraphEditor() = default;
	virtual ~DeferredRenderGraphEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onRenderGraphGUI(EditorInfo& info);
protected:
	DeferredRenderGraph* deferredRenderGraph = NULL;
};