#pragma once

#include "../Editor.h"
#include "../../ProbeSystem/CaptureProbeRender.h"
#include "../../Base.h"

class CaptureProbeRenderEditor : public Editor
{
public:
	CaptureProbeRenderEditor() = default;
	virtual ~CaptureProbeRenderEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual Texture2D* getIcon();

	virtual void onPersistentGizmo(GizmoInfo& info, Base* ownedObject);
	virtual void onHandleGizmo(GizmoInfo& info, Base* ownedObject);

	virtual void onCaptureProbeRenderGUI(EditorInfo & info);

	virtual void onGUI(EditorInfo & info);
protected:
	CaptureProbeRender* captureProbeRender = NULL;
};