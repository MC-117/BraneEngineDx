#include "EnvLightCaptureProbeRenderEditor.h"

RegistEditor(EnvLightCaptureProbeRender);

Texture2D* EnvLightCaptureProbeRenderEditor::icon = NULL;
Material* EnvLightCaptureProbeRenderEditor::visualMaterial = NULL;
Mesh* EnvLightCaptureProbeRenderEditor::sphereMesh = NULL;
bool EnvLightCaptureProbeRenderEditor::isInited = false;

void EnvLightCaptureProbeRenderEditor::setInspectedObject(void* object)
{
	envLightCaptureProbeRender = dynamic_cast<EnvLightCaptureProbeRender*>((CaptureProbeRender*)object);
	CaptureProbeRenderEditor::setInspectedObject(envLightCaptureProbeRender);
}

Texture2D* EnvLightCaptureProbeRenderEditor::getIcon()
{
	if (icon == NULL)
		icon = getAssetByPath<Texture2D>("Engine/Icons/ReflectionProbe_Icon.png");
	return icon;
}

void EnvLightCaptureProbeRenderEditor::onCaptureProbeRenderGUI(EditorInfo& info)
{
	CaptureProbeRenderEditor::onCaptureProbeRenderGUI(info);
	Color tintColor = envLightCaptureProbeRender->tintColor;
	if (ImGui::ColorEdit4("TintColor", (float*)&tintColor)) {
		envLightCaptureProbeRender->tintColor = tintColor;
	}

	float falloff = envLightCaptureProbeRender->falloff;
	if (ImGui::DragFloat("Falloff", &falloff, 0.01f)) {
		falloff = max(falloff, 0.0f);
		envLightCaptureProbeRender->falloff = falloff;
	}

	float cutoff = envLightCaptureProbeRender->cutoff;
	if (ImGui::DragFloat("Cutoff", &cutoff, 0.01f)) {
		cutoff = max(cutoff, 0.0f);
		envLightCaptureProbeRender->cutoff = cutoff;
	}

	int probeIndex = envLightCaptureProbeRender->getProbeIndex();
	if (probeIndex >= 0) {
		loadDefaultResource();
		Matrix4f T = Matrix4f::Identity();
		T.block(0, 3, 3, 1) = envLightCaptureProbeRender->getWorldPosition();
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

void EnvLightCaptureProbeRenderEditor::loadDefaultResource()
{
	if (isInited)
		return;
	visualMaterial = getAssetByPath<Material>("Engine/Shaders/Editor/VisualEnvLightProbe.mat");
	sphereMesh = getAssetByPath<Mesh>("Engine/Shapes/UnitSphere.fbx");
	if (visualMaterial == NULL || sphereMesh == NULL)
		throw runtime_error("Failed to load default resource");
	isInited = true;
}
