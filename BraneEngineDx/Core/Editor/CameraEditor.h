#pragma once

#include "ActorEditor.h"
#include "../Camera.h"

class CameraEditor : public TransformEditor
{
public:
	CameraEditor() = default;
	virtual ~CameraEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onHandleGizmo(GizmoInfo& info);

	virtual void onCameraGUI(EditorInfo& info);
	virtual void onAnimation(EditorInfo& info);
	virtual void onPostprocess(EditorInfo& info);

	virtual void onDetailGUI(EditorInfo& info);

	static GizmoCameraInfo getGizmoCameraInfo(Camera* camera);
protected:
	Camera* camera = NULL;
	GizmoCameraInfo gizmoCameraInfo;

	static Texture2D* icon;

	Texture2D* getIcon();
};