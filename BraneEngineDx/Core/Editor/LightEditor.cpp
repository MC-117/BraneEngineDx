#include "LightEditor.h"

RegistEditor(Light);

void LightEditor::setInspectedObject(void* object)
{
	light = dynamic_cast<Light*>((Object*)object);
	TransformEditor::setInspectedObject(light);
}

void LightEditor::onLightGUI(EditorInfo& info)
{
	Color color = light->color;
	if (ImGui::ColorEdit4("Color", (float*)&color))
		light->color = color;
	float val = light->intensity;
	if (ImGui::DragFloat("Intensity", &val, 0.01f))
		light->intensity = max(val, 0);
}

void LightEditor::onDetailGUI(EditorInfo& info)
{
	TransformEditor::onDetailGUI(info);
	if (ImGui::CollapsingHeader("Light"))
		onLightGUI(info);
}
