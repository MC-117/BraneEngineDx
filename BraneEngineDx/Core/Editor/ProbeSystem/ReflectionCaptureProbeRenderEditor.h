#pragma once

#include "CaptureProbeRenderEditor.h"
#include "../../ProbeSystem/ReflectionCaptureProbeRender.h"

class ReflectionCaptureProbeRenderEditor : public CaptureProbeRenderEditor
{
public:
	ReflectionCaptureProbeRenderEditor() = default;
	virtual ~ReflectionCaptureProbeRenderEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual Texture2D* getIcon();

	virtual void onCaptureProbeRenderGUI(EditorInfo & info);
protected:
	static Texture2D* icon;
	static Material* visualMaterial;
	static Mesh* sphereMesh;
	static bool isInited;

	ReflectionCaptureProbeRender* reflectionCaptureProbeRender = NULL;
	Material* instanceMaterial = NULL;

	static void loadDefaultResource();
};