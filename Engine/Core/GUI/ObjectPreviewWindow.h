#pragma once

#include "../Editor/EditorWorld.h"
#include "UIWindow.h"

class Asset;
class Gizmo;

class ENGINE_API ObjectPreviewWindow : public UIWindow
{
public:
	ObjectPreviewWindow(string name = "ObjectPreviewWindow", bool defaultShow = false);
	virtual ~ObjectPreviewWindow();

	virtual void setSerializable(const SerializationInfo& info);
	virtual void setAsset(Asset& asset);

	virtual void onUpdateScene(float width, float height);

	virtual void onToolBarGUI(GUIRenderInfo& info);
	virtual void onInspectorBarGUI(GUIRenderInfo& info);
	virtual void onSceneGUI(GUIRenderInfo& info, float width, float height);

	virtual void onWindowGUI(GUIRenderInfo& info);
	virtual void onRender(RenderInfo& info);

	static void showObject(GUI& gui, const SerializationInfo& info);
	static void showObject(GUI& gui, Asset& asset);
protected:
	EditorWorld editorWorld;
	Object* targetObject = NULL;

	float toolBarSizeRate = 0.15f;
	float inspectorSizeRate = 0.3f;
};
