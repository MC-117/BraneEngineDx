#include "CaptureProbeEditor.h"
#include "../../GUI/GUIUtility.h"
#include "../../GUI/GizmoUltility.h"

RegistEditor(CaptureProbe);

void CaptureProbeEditor::setInspectedObject(void* object)
{
	captureProbe = dynamic_cast<CaptureProbe*>((Object*)object);
	TransformEditor::setInspectedObject(captureProbe);
}

void CaptureProbeEditor::onPersistentGizmo(GizmoInfo& info)
{
	if (CaptureProbeRenderEditor* editor = getRenderEditor())
		editor->onPersistentGizmo(info, captureProbe);
}

void CaptureProbeEditor::onHandleGizmo(GizmoInfo& info)
{
	TransformEditor::onHandleGizmo(info);
	if (CaptureProbeRenderEditor* editor = getRenderEditor())
		editor->onHandleGizmo(info, captureProbe);
}

void CaptureProbeEditor::onProbeGUI(EditorInfo& info)
{
	Serialization* serialization = captureProbe->captureProbeRender ?
		&captureProbe->captureProbeRender->getSerialization() : NULL;
	if (ImGui::TypeCombo("ProbeType", serialization,
		CaptureProbeRender::CaptureProbeRenderSerialization::serialization)) {
		captureProbe->createCaptureProbeRender(*serialization);
	}
	if (CaptureProbeRenderEditor* editor = getRenderEditor())
		editor->onGUI(info);
}

void CaptureProbeEditor::onDetailGUI(EditorInfo& info)
{
	TransformEditor::onDetailGUI(info);
	if (ImGui::CollapsingHeader("ReflectionProbe"))
		onProbeGUI(info);
}

CaptureProbeRenderEditor* CaptureProbeEditor::getRenderEditor()
{
	if (captureProbe == NULL || captureProbe->captureProbeRender == NULL)
		return NULL;
	return dynamic_cast<CaptureProbeRenderEditor*>(
		EditorManager::getEditor(*captureProbe->captureProbeRender));
}
