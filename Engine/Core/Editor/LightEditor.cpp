#include "LightEditor.h"
#include "../GUI/GizmoUltility.h"

RegistEditor(Light);

void LightEditor::setInspectedObject(void* object)
{
	light = dynamic_cast<Light*>((Object*)object);
	TransformEditor::setInspectedObject(light);
}

void LightEditor::onPersistentGizmo(GizmoInfo& info)
{
	TransformEditor::onPersistentGizmo(info);

	Texture2D* icon = getIcon();

	Vector3f pos = light->getPosition(WORLD);

	if (icon != NULL) {
		info.gizmo->drawIcon(*icon, pos, getFitIconSize(info), light->color);
		if (info.gizmo->pickIcon(pos, Vector2f::Zero(),
			{ (float)icon->getWidth(), (float)icon->getHeight() }))
				EditorManager::selectObject(light);
	}
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

Texture2D* LightEditor::getIcon()
{
	return NULL;
}
