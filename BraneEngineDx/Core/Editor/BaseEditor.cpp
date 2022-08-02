#include "BaseEditor.h"

RegistEditor(Base);

BaseEditor::~BaseEditor()
{
	EditorManager::editors->erase(base);
}

void BaseEditor::setInspectedObject(void* object)
{
	base = (Base*)object;
	if (base != NULL)
		setInstanceParent(*base);
}

void BaseEditor::onInstanceGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("Instance Info")) {
		InstanceAssetHandle handle = base->getAssetHandle();
		InstanceAssetFile assetFile = Brane::getAssetFileByFileID(handle.fileID);
		ImGui::Text("InstanceID: %lld", base->getInstanceID());
		ImGui::Text("FileID: %d", handle.fileID);
		ImGui::Text("GUID: %s", handle.guid.toString().c_str());
		ImGui::Text("Path: %s", assetFile.path.c_str());

		if (ImGui::TreeNode("LinkedInstances")) {
			const list<InstanceID>& instances = base->getLinkedInstances();
			for (auto b = instances.begin(),
				e = instances.end(); b != e; b++) {
				ImGui::Text(to_string(*b).c_str());
			}
			ImGui::TreePop();
		}
	}
}

void BaseEditor::onGUI(EditorInfo& info)
{
	if (base == NULL)
		return;
	onInstanceGUI(info);
}
