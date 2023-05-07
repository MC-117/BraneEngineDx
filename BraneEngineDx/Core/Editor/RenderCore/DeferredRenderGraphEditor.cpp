#include "DeferredRenderGraphEditor.h"
#include "../../Profile/ProfileCore.h"
#include "../../GUI/GUIUtility.h"

RegistEditor(DeferredRenderGraph);

void DeferredRenderGraphEditor::setInspectedObject(void* object)
{
	deferredRenderGraph = dynamic_cast<DeferredRenderGraph*>((RenderGraph*)object);
	RenderGraphEditor::setInspectedObject(deferredRenderGraph);
}

void DeferredRenderGraphEditor::onRenderGraphGUI(EditorInfo& info)
{
	ImGui::Checkbox("EnablePreDepth", &deferredRenderGraph->enablePreDepthPass);
	ImGui::BeginGroupPanel("VSM");
	bool enableVSM = VirtualShadowMapConfig::isEnable();
	if (ImGui::Checkbox("EnableVSM", &enableVSM)) {
		VirtualShadowMapConfig::setEnable(enableVSM);
		/*if (enableVSM)
			ProfilerManager::instance().setNextCapture();*/
	}

	VirtualShadowMapConfig& config = VirtualShadowMapConfig::instance();
	const char* debugViewModeNames[] = {
		"None",
		"ClipmapLevel",
		"Clipmap"
	};
	if (ImGui::BeginCombo("VSMDebugViewMode", debugViewModeNames[config.debugViewMode])) {
		for (int i = 0; i < VirtualShadowMapConfig::DebugViewMode::Num; i++) {
			if (ImGui::Selectable(debugViewModeNames[i], i == config.debugViewMode)) {
				config.debugViewMode = i;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(config.debugViewMode != VirtualShadowMapConfig::DebugViewMode::Clipmap);
	ImGui::DragInt("VSMID", &config.debugVSMID, 1, 0, config.lastClipmapLevel - config.firstClipmapLevel);
	ImGui::EndDisabled();
	ImGui::EndGroupPanel();
	
	ImGui::Checkbox("EnableSSR", &deferredRenderGraph->ssrPass.enable);
	RenderGraphEditor::onRenderGraphGUI(info);
}
