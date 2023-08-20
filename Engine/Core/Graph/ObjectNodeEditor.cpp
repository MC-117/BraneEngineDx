#include "ObjectNodeEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(RefPin);

void RefPinEditor::setInspectedObject(void* object)
{
	refPin = dynamic_cast<RefPin*>((Base*)object);
	GraphPinEditor::setInspectedObject(refPin);
}

void RefPinEditor::onInspectGUI(EditorInfo& info)
{
	const Serialization& type = refPin->getRefSerialization();
	if (&type == &Object::ObjectSerialization::serialization ||
		type.isChildOf(Object::ObjectSerialization::serialization)) {
		Object* object = dynamic_cast<Object*>(refPin->getBase());
		if (ImGui::ObjectCombo(type.type.c_str(), object, info.world->getObject(), type.type)) {
			refPin->castFromInternal(object);
		}
	}
}

void RefPinEditor::onPinGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (refPin->getConnectedPin())
		GraphPinEditor::onPinGUI(info, graphInfo);
	else {
		if (&refPin->getSerialization() == &ObjectRefPin::ObjectRefPinSerialization::serialization) {
			Object* object = dynamic_cast<Object*>(refPin->getBase());
			ImGui::SetNextItemWidth(100);
			if (ImGui::ObjectCombo("Object", object, info.world->getObject(), refPin->getRefSerialization().type)) {
				refPin->castFromInternal(object);
			}
		}
	}
}

IMP_VAR_EDITOR(ObjectRefVariable);

RegistEditor(RefVariable);

void RefVariableEditor::setInspectedObject(void* object)
{
	valueVariable = dynamic_cast<RefVariable*>((Base*)object);
	GraphBaseEditor::setInspectedObject(valueVariable);
}

void RefVariableEditor::onInspectGUI(EditorInfo& info)
{
	const Serialization& type = valueVariable->getRefSerialization();
	if (&type == &Object::ObjectSerialization::serialization ||
		type.isChildOf(Object::ObjectSerialization::serialization)) {
		Object* object = dynamic_cast<Object*>(valueVariable->getBase());
		if (ImGui::ObjectCombo(type.type.c_str(), object, info.world->getObject(), type.type)) {
			valueVariable->setBase(object);
		}
		object = dynamic_cast<Object*>(valueVariable->getDefaultBase());
		ImGui::PushID("Default");
		ImGui::Text("Default");
		if (ImGui::ObjectCombo(type.type.c_str(), object, info.world->getObject(), type.type)) {
			valueVariable->setDefaultBase(object);
		}
		ImGui::PopID();
		if (ImGui::Button("ResetToDefault")) {
			valueVariable->resetToDefault();
		}
	}
}