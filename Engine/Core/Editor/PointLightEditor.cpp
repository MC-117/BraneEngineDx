#include "PointLightEditor.h"
#include "../GUI/GizmoUltility.h"
#include "Core/Camera.h"

RegistEditor(PointLight);

Texture2D* PointLightEditor::pointLightIcon = NULL;

void PointLightEditor::setInspectedObject(void* object)
{
	pointLight = dynamic_cast<PointLight*>((Object*)object);
	LightEditor::setInspectedObject(pointLight);
}

void PointLightEditor::onHandleGizmo(GizmoInfo& info)
{
	LightEditor::onHandleGizmo(info);
	onLocalLightShapeGizmo(info);
	info.camera->cameraRender.setDebugProbeIndex(pointLight->getProbeIndex());
}

void PointLightEditor::onLocalLightShapeGizmo(GizmoInfo& info)
{
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
	if (pointLightIcon == NULL)
		pointLightIcon = getAssetByPath<Texture2D>("Engine/Icons/PointLight_Icon.png");
	return pointLightIcon;
}
