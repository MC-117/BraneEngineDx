#pragma once

#include "CaptureProbeRenderEditor.h"
#include "../../ProbeSystem/EnvLightCaptureProbeRender.h"

class EnvLightCaptureProbeRenderEditor : public CaptureProbeRenderEditor
{
public:
	EnvLightCaptureProbeRenderEditor() = default;
	virtual ~EnvLightCaptureProbeRenderEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual Texture2D* getIcon();

	virtual void onCaptureProbeRenderGUI(EditorInfo& info);
protected:
	static Texture2D* icon;
	static Material* visualMaterial;
	static Mesh* sphereMesh;
	static bool isInited;

	EnvLightCaptureProbeRender* envLightCaptureProbeRender = NULL;
	Material* instanceMaterial = NULL;

	static void loadDefaultResource();
};