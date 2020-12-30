#include "SerializationEditor.h"
#include "../Engine.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"

SerializationEditor::SerializationEditor(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void SerializationEditor::setSerializtionInfo(SerializationInfo & info)
{
	this->serializationInfo = &info;
}

void SerializationEditor::onRenderWindow(GUIRenderInfo & info)
{
	drawInfo(serializationInfo);
}

void SerializationEditor::showSerializationInfo(GUI& gui, SerializationInfo & info)
{
	SerializationEditor *win = dynamic_cast<SerializationEditor*>(gui.getUIControl("SerializationEditor"));
	if (win == NULL) {
		win = new SerializationEditor();
		gui.addUIControl(*win);
	}
	win->show = true;
	win->setSerializtionInfo(info);
}

void SerializationEditor::drawInfo(SerializationInfo * info)
{
	if (info == NULL)
		return;
	string name = info->name + ": " + info->type;
	ImGui::PushID(info);
	bool open = ImGui::TreeNode(name.c_str());
	if (info->serialization != NULL) {
		ImGui::SameLine();
		if (ImGui::Button("Add to World")) {
			Serializable* ser = info->serialization->instantiate(*info);
			Object* obj = dynamic_cast<Object*>(ser);
			if (obj == NULL) {
				if (ser != NULL)
					delete ser;
			}
			else {
				obj->deserialize(*info);
				object.addChild(*obj);
			}
		}
	}
	if (open) {
		ImGui::PushID(0);
		for (auto b = info->numFeild.begin(), e = info->numFeild.end(); b != e; b++) {
			float num = info->numList[b->second];
			if (ImGui::InputFloat(b->first.c_str(), &num))
				info->numList[b->second] = num;
		}
		ImGui::PopID();
		ImGui::PushID(1);
		for (auto b = info->stringFeild.begin(), e = info->stringFeild.end(); b != e; b++) {
			string str = info->stringList[b->second];
			if (ImGui::InputText(b->first.c_str(), &str))
				info->stringList[b->second] = str;
		}
		ImGui::PopID();
		for (auto b = info->subfeilds.begin(), e = info->subfeilds.end(); b != e; b++) {
			drawInfo(&info->sublists[b->second]);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}
