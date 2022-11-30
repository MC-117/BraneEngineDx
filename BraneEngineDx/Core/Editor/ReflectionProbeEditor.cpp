#include "ReflectionProbeEditor.h"
#include "../GUI/GizmoUltility.h"

RegistEditor(ReflectionProbe);

Texture2D* ReflectionProbeEditor::icon = NULL;
Material* ReflectionProbeEditor::visualMaterial = NULL;
Mesh* ReflectionProbeEditor::sphereMesh = NULL;
bool ReflectionProbeEditor::isInited = false;

void ReflectionProbeEditor::setInspectedObject(void* object)
{
	reflectionProbe = dynamic_cast<ReflectionProbe*>((Object*)object);
	TransformEditor::setInspectedObject(reflectionProbe);
}

void ReflectionProbeEditor::onPersistentGizmo(GizmoInfo& info)
{
	Texture2D* icon = getIcon();

	Vector3f pos = reflectionProbe->getPosition(WORLD);

	if (icon != NULL) {
		info.gizmo->drawIcon(*icon, pos, getFitIconSize(info), reflectionProbe->capture.tintColor);
		if (info.gizmo->pickIcon(pos, Vector2f::Zero(),
			{ (float)icon->getWidth(), (float)icon->getHeight() }))
			EditorManager::selectObject(reflectionProbe);
	}
}

void ReflectionProbeEditor::onHandleGizmo(GizmoInfo& info)
{
	TransformEditor::onHandleGizmo(info);
	info.gizmo->drawSphere(reflectionProbe->capture.getWorldPosition(), reflectionProbe->capture.getRadius(), Matrix4f::Identity(), reflectionProbe->capture.tintColor);
}

void ReflectionProbeEditor::onReflectionProbeGUI(EditorInfo& info)
{
	float radius = reflectionProbe->capture.getRadius();
	if (ImGui::DragFloat("Radius", &radius, 0.01f)) {
		radius = max(radius, 0);
		reflectionProbe->capture.setRadius(radius);
	}
	if (ImGui::Button("UpdateCapture", { -1, 36 })) {
		reflectionProbe->capture.updateCapture();
	}
	int probeIndex = reflectionProbe->capture.getProbeIndex();
	if (probeIndex >= 0) {
		loadDefaultResource();
		Matrix4f T = Matrix4f::Identity();
		T.block(0, 3, 3, 1) = reflectionProbe->capture.getWorldPosition();
		Matrix4f S = Matrix4f::Identity();
		S(0, 0) = 20;
		S(1, 1) = 20;
		S(2, 2) = 20;
		Matrix4f transformMat = T * S;
		if (instanceMaterial == NULL)
			instanceMaterial = &visualMaterial->instantiate();
		instanceMaterial->setCount("probeIndex", probeIndex);
		info.gizmo->drawMesh(sphereMesh->meshParts[0], *instanceMaterial, transformMat);
	}
}

void ReflectionProbeEditor::onDetailGUI(EditorInfo& info)
{
	TransformEditor::onDetailGUI(info);
	if (ImGui::CollapsingHeader("ReflectionProbe"))
		onReflectionProbeGUI(info);
}

Texture2D* ReflectionProbeEditor::getIcon()
{
	if (icon == NULL)
		icon = getAssetByPath<Texture2D>("Engine/Icons/ReflectionProbe_Icon.png");
	return icon;
}

void ReflectionProbeEditor::loadDefaultResource()
{
	if (isInited)
		return;
	visualMaterial = getAssetByPath<Material>("Engine/Shaders/Debug/VisualReflectionProbe.mat");
	sphereMesh = getAssetByPath<Mesh>("Engine/Shapes/UnitSphere.fbx");
	if (visualMaterial == NULL || sphereMesh == NULL)
		throw runtime_error("Failed to load default resource");
	isInited = true;
}
