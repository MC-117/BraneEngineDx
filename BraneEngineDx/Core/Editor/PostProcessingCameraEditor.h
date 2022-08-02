#pragma once

#include "CameraEditor.h"
#include "../PostProcess/PostProcessingCamera.h"

class PostProcessingCameraEditor : public CameraEditor
{
public:
	PostProcessingCameraEditor() = default;
	virtual ~PostProcessingCameraEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPostprocess(EditorInfo& info);
protected:
	PostProcessingCamera* postProcessingCamera = NULL;
};