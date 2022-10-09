#include "SerializationEditor.h"
#include "../Engine.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "GUIUtility.h"

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

bool SerializationEditor::drawInfo(SerializationInfo * info)
{
	bool doDelete = false;
	if (info == NULL)
		return doDelete;
	string name = info->name + ": " + info->type;
	ImGui::PushID(info);
	bool open = ImGui::TreeNode(name.c_str());
	if (info->serialization != NULL) {
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_FILE_IMPORT)) {
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
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_BACKSPACE)) {
		doDelete = true;
	}
	if (open) {
		ImGui::PushID(0);
		for (auto b = info->numFeild.begin(), e = info->numFeild.end(); b != e;) {
			float num = info->numList[b->second];
			if (ImGui::InputFloat(b->first.c_str(), &num))
				info->numList[b->second] = num;
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_CROSS)) {
				int index = b->second;
				info->numList.erase(info->numList.begin() += index);
				b = info->numFeild.erase(b);
				for (auto _b = info->numFeild.begin(), _e = info->numFeild.end(); _b != _e; _b++)
					if (_b->second > index)
						_b->second--;
			}
			else b++;
		}
		ImGui::PopID();
		ImGui::PushID(1);
		for (auto b = info->stringFeild.begin(), e = info->stringFeild.end(); b != e;) {
			string str = info->stringList[b->second];
			if (ImGui::InputText(b->first.c_str(), &str))
				info->stringList[b->second] = str;
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_CROSS)) {
				int index = b->second;
				info->stringList.erase(info->stringList.begin() += index);
				b = info->stringFeild.erase(b);
				for (auto _b = info->stringFeild.begin(), _e = info->stringFeild.end(); _b != _e; _b++)
					if (_b->second > index)
						_b->second--;
			}
			else b++;
		}
		ImGui::PopID();
		for (auto b = info->subfeilds.begin(), e = info->subfeilds.end(); b != e;) {
			if (drawInfo(&info->sublists[b->second])) {
				int index = b->second;
				info->sublists.erase(info->sublists.begin() += index);
				b = info->subfeilds.erase(b);
				for (auto _b = info->subfeilds.begin(), _e = info->subfeilds.end(); _b != _e; _b++)
					if (_b->second > index)
						_b->second--;
			}
			else b++;
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
	return doDelete;
}
