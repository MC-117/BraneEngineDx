#include "DeferredRenderGraphEditor.h"

RegistEditor(DeferredRenderGraph);

void DeferredRenderGraphEditor::setInspectedObject(void* object)
{
	deferredRenderGraph = dynamic_cast<DeferredRenderGraph*>((RenderGraph*)object);
	RenderGraphEditor::setInspectedObject(deferredRenderGraph);
}

void DeferredRenderGraphEditor::onRenderGraphGUI(EditorInfo& info)
{
	ImGui::Checkbox("EnableSSR", &deferredRenderGraph->ssrPass.enable);
	RenderGraphEditor::onRenderGraphGUI(info);
}
