#pragma once

#include "../TransformEditor.h"
#include "CaptureProbeRenderEditor.h"
#include "../../ProbeSystem/CaptureProbe.h"

class CaptureProbeEditor : public TransformEditor
{
public:
	CaptureProbeEditor() = default;
	virtual ~CaptureProbeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onHandleGizmo(GizmoInfo& info);

	virtual void onProbeGUI(EditorInfo& info);

	virtual void onDetailGUI(EditorInfo& info);
protected:
	CaptureProbe* captureProbe = NULL;

	CaptureProbeRenderEditor* getRenderEditor();
};