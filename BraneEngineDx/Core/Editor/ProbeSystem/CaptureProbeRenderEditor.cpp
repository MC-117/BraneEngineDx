#include "CaptureProbeRenderEditor.h"
#include "../../Transform.h"
#include "../../GUI/GizmoUltility.h"

RegistEditor(CaptureProbeRender);

void CaptureProbeRenderEditor::setInspectedObject(void* object)
{
	captureProbeRender = static_cast<CaptureProbeRender*>(object);
}

Texture2D* CaptureProbeRenderEditor::getIcon()
{
	return NULL;
}

void CaptureProbeRenderEditor::onPersistentGizmo(GizmoInfo& info, Base* ownedObject)
{
	Transform* transform = dynamic_cast<Transform*>(ownedObject);
	if (transform == NULL)
		return;
	Texture2D* icon = getIcon();

	Vector3f pos = transform->getPosition(WORLD);

	if (icon != NULL) {
		info.gizmo->drawIcon(*icon, pos, getFitIconSize(info));
		if (info.gizmo->pickIcon(pos, Vector2f::Zero(),
			{ (float)icon->getWidth(), (float)icon->getHeight() }))
			EditorManager::selectObject(transform);
	}
}

void CaptureProbeRenderEditor::onHandleGizmo(GizmoInfo& info, Base* ownedObject)
{
	info.gizmo->drawSphere(captureProbeRender->getWorldPosition(),
		captureProbeRender->getRadius(), Matrix4f::Identity(), { 1.0f, 1.0f, 1.0f, 1.0f });
}

void CaptureProbeRenderEditor::onCaptureProbeRenderGUI(EditorInfo& info)
{
	if (ImGui::Button("UpdateCapture", { -1, 36 })) {
		captureProbeRender->updateCapture();
	}

	float radius = captureProbeRender->getRadius();
	if (ImGui::DragFloat("Radius", &radius, 0.01f)) {
		radius = max(radius, 0);
		captureProbeRender->setRadius(radius);
	}
	int curResolution = captureProbeRender->getResolution();
	string resolutionStr = to_string(curResolution);
	const int resolutions[] = { 64, 128, 256, 512, 1024, 2048, 4096 };
	if (ImGui::BeginCombo("Resolution", resolutionStr.c_str())) {
		for (int i = 0; i < 7; i++) {
			curResolution = resolutions[i];
			resolutionStr = to_string(curResolution);
			if (ImGui::Selectable(resolutionStr.c_str()))
				captureProbeRender->setResolution(curResolution);
		}
		ImGui::EndCombo();
	}
}

void CaptureProbeRenderEditor::onGUI(EditorInfo& info)
{
	if (captureProbeRender == NULL)
		return;
	onCaptureProbeRenderGUI(info);
}
