#pragma once
#ifndef _MATERIALWINDOW_H_
#define _MATERIALWINDOW_H_

#include "UIWindow.h"
#include "../Editor/EditorWorld.h"
#include "../MeshActor.h"

class MaterialWindow : public UIWindow
{
public:
	MaterialWindow(Material* material = NULL, string name = "MaterialWindow", bool defaultShow = false);

	void setMaterial(Material& material);
	Material* getMaterial();

	void onMaterialPreview(GUIRenderInfo& info);
	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	Material* material = NULL;
	EditorWorld editorWorld;
	Gizmo gizmo;
	MeshActor meshActor;
};

#endif // !_MATERIALWINDOW_H_