#pragma once

#include "UIWindow.h"
#include "../Object.h"
#include "../Camera.h"
#include "../MeshRender.h"
#include "../Asset.h"

class ENGINE_API UVViewer : public UIWindow
{
public:
	UVViewer(string name = "UVViewer", bool defaultShow = false);

	void setTargetMeshPart(Mesh* mesh);

	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	Texture2D uvTexture = Texture2D(128, 128, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear });
	RenderTarget renderTarget = RenderTarget(128, 128, 4);
	Material* uvMaterial;
	Camera camera;

	int partIndex = 0;
	Asset* meshAsset = NULL;
	Mesh* selectMesh = NULL;
	MeshPart* selectMeshPart = NULL;
	Asset* textureAsset = NULL;
	Texture2D* selectTexture = NULL;
};