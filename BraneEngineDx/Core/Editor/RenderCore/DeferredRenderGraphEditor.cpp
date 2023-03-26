#include "DeferredRenderGraphEditor.h"
#include "../../Profile/ProfileCore.h"

RegistEditor(DeferredRenderGraph);

void DeferredRenderGraphEditor::setInspectedObject(void* object)
{
	deferredRenderGraph = dynamic_cast<DeferredRenderGraph*>((RenderGraph*)object);
	RenderGraphEditor::setInspectedObject(deferredRenderGraph);
}

void DeferredRenderGraphEditor::onRenderGraphGUI(EditorInfo& info)
{
	ImGui::Checkbox("EnablePreDepth", &deferredRenderGraph->enablePreDepthPass);
	bool enableVSM = VirtualShadowMapConfig::isEnable();
	if (ImGui::Checkbox("EnableVSM", &enableVSM)) {
		VirtualShadowMapConfig::setEnable(enableVSM);
		if (enableVSM)
			ProfilerManager::instance().setNextCapture();
	}
	ImGui::Checkbox("EnableVSMDebug", &VirtualShadowMapConfig::instance().debugView);
	ImGui::Checkbox("EnableSSR", &deferredRenderGraph->ssrPass.enable);
	RenderGraphEditor::onRenderGraphGUI(info);
}
