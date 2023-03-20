#include "DeferredRenderGraphEditor.h"

RegistEditor(DeferredRenderGraph);

void DeferredRenderGraphEditor::setInspectedObject(void* object)
{
	deferredRenderGraph = dynamic_cast<DeferredRenderGraph*>((RenderGraph*)object);
	RenderGraphEditor::setInspectedObject(deferredRenderGraph);
}

void DeferredRenderGraphEditor::onRenderGraphGUI(EditorInfo& info)
{
	ImGui::Checkbox("EnablePreDepth", &deferredRenderGraph->enablePreDepthPass);
	ImGui::Checkbox("EnableVSM", &deferredRenderGraph->enableVSMDepthPass);
	ImGui::Checkbox("EnableSSR", &deferredRenderGraph->ssrPass.enable);
	RenderGraphEditor::onRenderGraphGUI(info);
}
