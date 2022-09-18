#include "DirectLightEditor.h"
#include "../GUI/GizmoUltility.h"

RegistEditor(DirectLight);

Texture2D* DirectLightEditor::icon = NULL;

void DirectLightEditor::setInspectedObject(void* object)
{
	directLight = dynamic_cast<DirectLight*>((Object*)object);
	LightEditor::setInspectedObject(directLight);
}

void DirectLightEditor::onPersistentGizmo(GizmoInfo& info)
{
	Texture2D* icon = getIcon();

	Vector3f dirPos = directLight->getPosition(WORLD);
	Vector3f dirFW = directLight->getForward(WORLD);

	if (icon != NULL) {
		info.gizmo->drawIcon(*icon, dirPos, getFitIconSize(info), directLight->color);
		if (info.gizmo->pickIcon(dirPos, Vector2f::Zero(),
			{ (float)icon->getWidth(), (float)icon->getHeight() }))
			EditorManager::selectObject(directLight);
	}
	info.gizmo->drawLine(dirPos, dirPos + dirFW * 10, directLight->color);
}

void DirectLightEditor::onLightGUI(EditorInfo& info)
{
	LightEditor::onLightGUI(info);
	float bias = directLight->getShadowBiasDepthScale();
	if (ImGui::DragFloat("ShadowBiasDepthScale", &bias, 0.01)) {
		directLight->setShadowBiasDepthScale(bias);
	}
	bias = directLight->getShadowBiasNormalScale();
	if (ImGui::DragFloat("ShadowBiasNormalScale", &bias, 0.01)) {
		directLight->setShadowBiasNormalScale(bias);
	}
}

Texture2D* DirectLightEditor::getIcon()
{
	if (icon == NULL)
		icon = getAssetByPath<Texture2D>("Engine/Icons/DirectLight_Icon.png");
	return icon;
}
