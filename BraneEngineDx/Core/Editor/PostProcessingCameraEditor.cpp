#include "PostProcessingCameraEditor.h"

RegistEditor(PostProcessingCamera);

void PostProcessingCameraEditor::setInspectedObject(void* object)
{
	postProcessingCamera = dynamic_cast<PostProcessingCamera*>((Object*)object);
	CameraEditor::setInspectedObject(postProcessingCamera);
}

void PostProcessingCameraEditor::onPostprocess(EditorInfo& info)
{
	PostProcessGraph& graph = postProcessingCamera->postProcessCameraRender.graph;
	ImGui::Checkbox("AutoDOF", &postProcessingCamera->autoDof);
	int i = 0;
	for (auto b = graph.passes.begin(), e = graph.passes.end(); b != e; b++, i++) {
		ImGui::PushID(i);
		bool enable = (*b)->getEnable();
		if (ImGui::Checkbox("##Enable", &enable)) {
			(*b)->setEnable(enable);
		}
		ImGui::SameLine();
		if (ImGui::CollapsingHeader((*b)->getName().c_str())) {
			ImGui::Indent(20);
			Editor* editor = EditorManager::getEditor("Material", (*b)->getMaterial());
			if (editor != NULL) {
				editor->onGUI(info);
			}
			ImGui::Unindent(20);
		}
		ImGui::PopID();
	}
}
