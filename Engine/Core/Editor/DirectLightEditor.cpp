#include "DirectLightEditor.h"
#include "../GUI/GizmoUltility.h"

RegistEditor(DirectLight);

Texture2D* DirectLightEditor::directLightIcon = NULL;

void DirectLightEditor::setInspectedObject(void* object)
{
	directLight = dynamic_cast<DirectLight*>((Object*)object);
	LightEditor::setInspectedObject(directLight);
}

void DirectLightEditor::onPersistentGizmo(GizmoInfo& info)
{
	LightEditor::onPersistentGizmo(info);
	Vector3f dirPos = light->getPosition(WORLD);
	Vector3f dirFW = directLight->getForward(WORLD);
	info.gizmo->drawLine(dirPos, dirPos + dirFW * 10, directLight->color);
}

void DirectLightEditor::onLightGUI(EditorInfo& info)
{
	LightEditor::onLightGUI(info);
	float resScale = directLight->getShadowResolutionScale();
	if (ImGui::DragFloat("ShadowResolutionScale", &resScale, 0.01)) {
		directLight->scaleShadowResolution(resScale);
	}
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
	if (directLightIcon == NULL)
		directLightIcon = getAssetByPath<Texture2D>("Engine/Icons/DirectLight_Icon.png");
	return directLightIcon;
}
