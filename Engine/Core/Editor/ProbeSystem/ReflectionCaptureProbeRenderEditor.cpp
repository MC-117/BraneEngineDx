#include "ReflectionCaptureProbeRenderEditor.h"

RegistEditor(ReflectionCaptureProbeRender);

Texture2D* ReflectionCaptureProbeRenderEditor::icon = NULL;
Material* ReflectionCaptureProbeRenderEditor::visualMaterial = NULL;
Mesh* ReflectionCaptureProbeRenderEditor::sphereMesh = NULL;
bool ReflectionCaptureProbeRenderEditor::isInited = false;

void ReflectionCaptureProbeRenderEditor::setInspectedObject(void* object)
{
	reflectionCaptureProbeRender = static_cast<ReflectionCaptureProbeRender*>((CaptureProbeRender*)object);
	CaptureProbeRenderEditor::setInspectedObject(reflectionCaptureProbeRender);
}

Texture2D* ReflectionCaptureProbeRenderEditor::getIcon()
{
	if (icon == NULL)
		icon = getAssetByPath<Texture2D>("Engine/Icons/ReflectionProbe_Icon.png");
	return icon;
}

void ReflectionCaptureProbeRenderEditor::onCaptureProbeRenderGUI(EditorInfo& info)
{
	CaptureProbeRenderEditor::onCaptureProbeRenderGUI(info);
	Color tintColor = reflectionCaptureProbeRender->tintColor;
	if (ImGui::ColorEdit4("TintColor", (float*)&tintColor)) {
		reflectionCaptureProbeRender->tintColor = tintColor;
	}

	float falloff = reflectionCaptureProbeRender->falloff;
	if (ImGui::DragFloat("Falloff", &falloff, 0.01f)) {
		falloff = max(falloff, 0);
		reflectionCaptureProbeRender->falloff = falloff;
	}

	float cutoff = reflectionCaptureProbeRender->cutoff;
	if (ImGui::DragFloat("Cutoff", &cutoff, 0.01f)) {
		cutoff = max(cutoff, 0);
		reflectionCaptureProbeRender->cutoff = cutoff;
	}

	int probeIndex = reflectionCaptureProbeRender->getProbeIndex();
	if (probeIndex >= 0) {
		loadDefaultResource();
		Matrix4f T = Matrix4f::Identity();
		T.block(0, 3, 3, 1) = reflectionCaptureProbeRender->getWorldPosition();
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

void ReflectionCaptureProbeRenderEditor::loadDefaultResource()
{
	if (isInited)
		return;
	visualMaterial = getAssetByPath<Material>("Engine/Shaders/Editor/VisualReflectionProbe.mat");
	sphereMesh = getAssetByPath<Mesh>("Engine/Shapes/UnitSphere.fbx");
	if (visualMaterial == NULL || sphereMesh == NULL)
		throw runtime_error("Failed to load default resource");
	isInited = true;
}
