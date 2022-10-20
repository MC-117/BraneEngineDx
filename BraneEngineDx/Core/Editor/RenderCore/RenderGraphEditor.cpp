#include "RenderGraphEditor.h"

RegistEditor(RenderGraph);

void RenderGraphEditor::setInspectedObject(void* object)
{
	renderGraph = dynamic_cast<RenderGraph*>((Serializable*)object);
}

void RenderGraphEditor::onRenderGraphGUI(EditorInfo& info)
{
	vector<pair<string, RenderPass*>> passes;
	renderGraph->getPasses(passes);
	for (int i = 0; i < passes.size(); i++) {
		ImGui::PushID(i);
		auto& pass = passes[i];
		if (ImGui::CollapsingHeader(pass.first.c_str())) {
			RenderPassEditor* editor = dynamic_cast<RenderPassEditor*>(
				EditorManager::getEditor("RenderPass", pass.second));
			if (editor) {
				editor->onGUI(info);
			}
		}
		ImGui::PopID();
	}
}

void RenderGraphEditor::onGUI(EditorInfo& info)
{
	if (renderGraph == NULL)
		return;
	onRenderGraphGUI(info);
}
