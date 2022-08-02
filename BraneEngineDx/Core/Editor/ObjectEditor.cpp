#include "ObjectEditor.h"
#include <thread>
#include <fstream>
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "../Console.h"
#include "../GUI/GUIUtility.h"
#include "../Graph/GraphWindow.h"

RegistEditor(ObjectBehavior);

void ObjectBehaviorEditor::setInspectedObject(void* object)
{
	behavior = dynamic_cast<ObjectBehavior*>((Base*)object);
	BaseEditor::setInspectedObject(behavior);
}

void ObjectBehaviorEditor::onBehaviorGUI(EditorInfo& info)
{
}

void ObjectBehaviorEditor::onGUI(EditorInfo& info)
{
	if (behavior == NULL)
		return;
	onBehaviorGUI(info);
}

RegistEditor(Object);

void ObjectEditor::setInspectedObject(void* object)
{
	this->object = dynamic_cast<Object*>((Base*)object);
	BaseEditor::setInspectedObject(this->object);
}

void ObjectEditor::onHeaderGUI(EditorInfo& info)
{
	ImGui::Text("Object Name: %s", object->name.c_str());
}

void ObjectEditor::onOperationGUI(EditorInfo& info)
{
	if (ImGui::Button("Copy", { -1, 36 })) {
		ImGui::SetClipboardText(object->name.c_str());
	}
	if (ImGui::Button("Destroy", { -1, 36 })) {
		object->destroy();
		if (EditorManager::getSelectedObject() == object)
			EditorManager::selectObject(NULL);
	}
	if (ImGui::Button("Load Profile", { -1, 36 })) {
		thread td = thread([](Object* tar) {
			FileDlgDesc desc;
			desc.title = "asset";
			desc.filter = "asset(*.asset)|*.asset";
			desc.initDir = "Content";
			desc.defFileExt = "asset";
			if (openFileDlg(desc)) {
				ifstream f = ifstream(desc.filePath);
				SerializationInfoParser parser = SerializationInfoParser(f);
				if (!parser.parse()) {
					MessageBox(NULL, "Load failed", "Error", MB_OK);
					Console::error("SerializationInfoParser error: %s", parser.parseError.c_str());
				}
				else if (!parser.infos.empty()) {
					if (!tar->deserialize(parser.infos[0])) {
						MessageBox(NULL, "Deserialize failed", "Error", MB_OK);
						Console::error("SerializationInfoParser error: %s", parser.parseError.c_str());
					}
				}
				f.close();
			}
		}, object);
		td.detach();
	}
	if (ImGui::Button("Save Profile", { -1, 40 })) {
		thread td = thread([](Object* tar) {
			FileDlgDesc desc;
			desc.title = "asset";
			desc.filter = "asset(*.asset)|*.asset";
			desc.initDir = "Content";
			desc.save = true;
			desc.defFileExt = "asset";
			if (openFileDlg(desc)) {
				if (AssetManager::saveAsset(*tar, desc.filePath) == NULL) {
					MessageBox(NULL, "Serialize failed", "Error", MB_OK);
					return;
				}
				else {
					MessageBox(NULL, "Complete", "Info", MB_OK);
				}
			}
		}, object);
		td.detach();
	}
	long long address = (long long)object;
	string addressStr = to_string(address);
	ImGui::InputText("Address", &addressStr);
	if (object->parent != NULL) {
		if (ImGui::Button("Duplicate", { -1, 36 })) {
			ImGui::OpenPopup("DuplicatePopup");
		}
		if (ImGui::BeginPopup("DuplicatePopup")) {
			ImGui::InputText("Name", &duplicateName);
			if (!duplicateName.empty() && object->parent->findChild(duplicateName) == NULL) {
				if (ImGui::Button("Create", { -1, 36 })) {
					SerializationInfo sinfo;
					if (object->serialize(sinfo)) {
						sinfo.name = duplicateName;
						Serialization* serialization = SerializationManager::getSerialization(sinfo.type);
						if (serialization != NULL) {
							Object* nobj = dynamic_cast<Object*>(serialization->instantiate(sinfo));
							if (nobj != NULL) {
								if (nobj->deserialize(sinfo)) {
									object->parent->addChild(*nobj);
								}
								else
									delete nobj;
							}
						}
					}
				}
			}
			ImGui::EndPopup();
		}
	}
}

void ObjectEditor::onBehaviorGUI(EditorInfo& info)
{
	ImGui::TypeCombo("Behavior", behaviorType, ObjectBehavior::ObjectBehaviorSerialization::serialization);
	if (ImGui::Button("AddBehavior")) {
		if (behaviorType) {
			Serializable* serializable = behaviorType->instantiate(SerializationInfo());
			if (serializable) {
				ObjectBehavior* behavior = dynamic_cast<ObjectBehavior*>(serializable);
				if (behavior) {
					object->addBehavior(behavior);
				}
				else {
					delete serializable;
				}
			}
		}
	}
	for (int i = 0; i < object->getBehaviorCount(); i++) {
		ObjectBehavior* behavior = object->getBehavior(i);
		ObjectBehaviorEditor* editor = dynamic_cast<ObjectBehaviorEditor*>(EditorManager::getEditor(*behavior));
		if (editor) {
			if (ImGui::BeginHeaderBox(behavior->getName().c_str())) {
				editor->onGUI(info);
			}
			else {
				object->removeBehavior(behavior);
			}
			ImGui::EndHeaderBox();
		}
	}
}

void ObjectEditor::onEventGUI(EditorInfo& info)
{
	for (auto b = object->events.voidFuncField.begin(), e = object->events.voidFuncField.end();
		b != e; b++) {
		if (ImGui::Button(b->first.c_str(), { -1, 36 }))
			b->second(object);
	}
	if (object->events.objectFuncField.size() != objects.size()) {
		objects.clear();
		objects.resize(object->events.objectFuncField.size());
	}
	int i = 0;
	for (auto b = object->events.objectFuncField.begin(), e = object->events.objectFuncField.end();
		b != e; b++, i++) {
		ImGui::PushID(i);
		Object*& selectObj = objects[i];
		ImGui::ObjectCombo("Object", selectObj, info.world->getObject(), objectFilterName);
		if (ImGui::Button(b->first.c_str(), { -1, 36 }))
			b->second(object, selectObj);
		ImGui::PopID();
	}
	if (object->events.intFuncField.size() != ints.size()) {
		ints.clear();
		ints.resize(object->events.intFuncField.size());
	}
	i = 0;
	for (auto b = object->events.intFuncField.begin(), e = object->events.intFuncField.end();
		b != e; b++, i++) {
		int value = ints[i];
		if (ImGui::InputInt(("Int##" + b->first + "Int").c_str(), &value))
			ints[i] = value;
		if (ImGui::Button(b->first.c_str(), { -1, 36 }))
			b->second(object, ints[i]);
	}
	if (object->events.floatFuncField.size() != floats.size()) {
		floats.clear();
		floats.resize(object->events.floatFuncField.size());
	}
	i = 0;
	for (auto b = object->events.floatFuncField.begin(), e = object->events.floatFuncField.end();
		b != e; b++, i++) {
		float value = floats[i];
		if (ImGui::InputFloat(("Float##" + b->first + "Float").c_str(), &value))
			floats[i] = value;
		if (ImGui::Button(b->first.c_str(), { -1, 36 }))
			b->second(object, floats[i]);
	}
	if (object->events.stringFuncField.size() != strings.size()) {
		strings.clear();
		strings.resize(object->events.stringFuncField.size());
	}
	i = 0;
	for (auto b = object->events.stringFuncField.begin(), e = object->events.stringFuncField.end();
		b != e; b++, i++) {
		ImGui::InputText(("String##" + b->first + "String").c_str(), &strings[i]);
		if (ImGui::Button(b->first.c_str(), { -1, 36 }))
			b->second(object, strings[i]);
	}
}

void ObjectEditor::onDetailGUI(EditorInfo& info)
{
	
}

void ObjectEditor::onGUI(EditorInfo& info)
{
	if (object == NULL)
		return;
	BaseEditor::onGUI(info);
	onHeaderGUI(info);
	onOperationGUI(info);
	if (ImGui::CollapsingHeader("Behaviors"))
		onBehaviorGUI(info);
	if (ImGui::CollapsingHeader("Events"))
		onEventGUI(info);
	onDetailGUI(info);
}
