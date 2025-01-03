#include "DeferredRenderGraphEditor.h"
#include "../../Profile/ProfileCore.h"
#include "../../GUI/GUIUtility.h"

RegistEditor(DeferredRenderGraph);

void DeferredRenderGraphEditor::setInspectedObject(void* object)
{
	deferredRenderGraph = dynamic_cast<DeferredRenderGraph*>((RenderGraph*)object);
	RenderGraphEditor::setInspectedObject(deferredRenderGraph);
}

//extern bool captureVSMTrigger;

void DeferredRenderGraphEditor::onRenderGraphGUI(EditorInfo& info)
{
	if (ImGui::Button("TriggerDebugDraw", { -1, 36 }))
		deferredRenderGraph->triggerPersistentDebugDraw();
	ImGui::Checkbox("EnablePreDepth", &deferredRenderGraph->enablePreDepthPass);
	
	ImGui::BeginGroupPanel("VSM");
	{
		bool enableVSM = VirtualShadowMapConfig::isEnable();
		if (ImGui::Checkbox("EnableVSM", &enableVSM)) {
			VirtualShadowMapConfig::setEnable(enableVSM);
			// captureVSMTrigger = true;
			/*if (enableVSM)
				ProfilerManager::instance().setNextCapture();*/
		}

		VirtualShadowMapConfig& config = VirtualShadowMapConfig::instance();

		ImGui::DragFloat("LocalMinZ", &config.localLightViewMinZ, 0.00005f, 0, 1, "%.5f");
		ImGui::DragFloat("ScreenRayLen", &config.screenRayLength, 0.00005f, 0, 1, "%.5f");
		ImGui::DragInt("pcfPixel", (int*)&config.pcfPixel, 0.00005f, 0, 5);
		ImGui::DragInt("pcfStep", (int*)&config.pcfStep, 0.00005f, 1, 5);
		ImGui::DragFloat("pcfRadiusScale", &config.pcfRadiusScale, 0.01f, 0, 5);

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
		// if (!deferredRenderGraph->sceneDatas.empty())
		// {
		// 	SceneRenderData* sceneData = NULL;
		// 	sceneData = *deferredRenderGraph->sceneDatas.begin();
		// 	int count = sceneData->virtualShadowMapRenderData.manager.getShadowMapCount();
		// 	ImGui::DragInt("VSMID", &config.debugVSMID, 1, 0, count);
		// }
		ImGui::EndDisabled();
		ImGui::EndGroupPanel();
	}

	ImGui::BeginGroupPanel("ProbeGrid");
	{
		ProbeGridConfig& config = ProbeGridConfig::instance();
		int selectPixels = config.probeGridPixels;
		if (ImGui::BeginCombo("GridPixels", to_string(selectPixels).c_str())) {
			for (int i = 16; i <= 512; i <<= 1) {
				if (ImGui::Selectable(to_string(i).c_str(), i == selectPixels)) {
					config.probeGridPixels = i;
				}
			}
			ImGui::EndCombo();
		}
		int selectZSlices = config.probeGridZSlices;
		if (ImGui::BeginCombo("GridZSlices", to_string(selectZSlices).c_str())) {
			for (int i = 16; i <= 512; i <<= 1) {
				if (ImGui::Selectable(to_string(i).c_str(), i == selectZSlices)) {
					config.probeGridZSlices = i;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::DragFloat("NearOffsetScale", &config.nearOffsetScale, 0.1f, 1);
	}
	ImGui::EndGroupPanel();
	
	ImGui::BeginGroupPanel("SSR");
	{
		ImGui::Checkbox("EnableSSR", &deferredRenderGraph->ssrPass.enable);
		ImGui::DragInt("HiZStartLevel", &deferredRenderGraph->ssrPass.ssrBinding.ssrInfo.hiZStartLevel, 1, 0);
		ImGui::DragInt("HiZStopLevel", &deferredRenderGraph->ssrPass.ssrBinding.ssrInfo.hiZStopLevel, 1, -1);
		ImGui::DragInt("HiZMaxStep", &deferredRenderGraph->ssrPass.ssrBinding.ssrInfo.hiZMaxStep, 1);
		ImGui::DragInt("ResolveSamples", &deferredRenderGraph->ssrPass.ssrBinding.ssrInfo.ssrResolveSamples, 1, 1);
		ImGui::DragFloat("ResolveRadius", &deferredRenderGraph->ssrPass.ssrBinding.ssrInfo.ssrResolveRadius, 0.001f);
	}
	ImGui::EndGroupPanel();
	RenderGraphEditor::onRenderGraphGUI(info);
}
