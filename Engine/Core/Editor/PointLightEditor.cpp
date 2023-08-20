#include "PointLightEditor.h"
#include "../GUI/GizmoUltility.h"

RegistEditor(PointLight);

Texture2D* PointLightEditor::icon = NULL;

void PointLightEditor::setInspectedObject(void* object)
{
	pointLight = dynamic_cast<PointLight*>((Object*)object);
	LightEditor::setInspectedObject(pointLight);
}

void PointLightEditor::onPersistentGizmo(GizmoInfo& info)
{
	Texture2D* icon = getIcon();

	Vector3f pos = pointLight->getPosition(WORLD);

	if (icon != NULL) {
		info.gizmo->drawIcon(*icon, pos, getFitIconSize(info), pointLight->color);
		if (info.gizmo->pickIcon(pos, Vector2f::Zero(),
			{ (float)icon->getWidth(), (float)icon->getHeight() }))
			EditorManager::selectObject(pointLight);
	}
}

void PointLightEditor::onHandleGizmo(GizmoInfo& info)
{
	LightEditor::onHandleGizmo(info);
	info.gizmo->drawSphere(Vector3f::Zero(), pointLight->getRadius(), pointLight->getTransformMat(), pointLight->color);
}

void PointLightEditor::onLightGUI(EditorInfo& info)
{
	LightEditor::onLightGUI(info);
	float radius = pointLight->getRadius();
	if (ImGui::DragFloat("Radius", &radius, 0.01f))
		pointLight->setRadius(radius);
}

Texture2D* PointLightEditor::getIcon()
{
	if (icon == NULL)
		icon = getAssetByPath<Texture2D>("Engine/Icons/PointLight_Icon.png");
	return icon;
}
