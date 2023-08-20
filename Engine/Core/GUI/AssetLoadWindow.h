#pragma once
#ifndef _ASSETLOADWINDOW_H_
#define _ASSETLOADWINDOW_H_

#include "UIWindow.h"
#include "../SkeletonMesh.h"
#include "../Material.h"

class AssetLoadWindow : public UIWindow
{
public:
	char path[200] = "\0";
	char name[100] = "\0";

	Material* selectedMat = NULL;
	Mesh* selectedMesh = NULL;
	SkeletonMesh* selectedSkeletonMesh = NULL;
	bool twoSides = false;
	bool castShadow = false;
	int phyMatCT = 0;
	float mass = 0;
	int selectedId = 0;
	int selectedMeshId = 0;
	bool willLoad = false;

	AssetLoadWindow(Object& object, string name = "LoadAsset", bool defaultShow = false);
	virtual void onRenderWindow(GUIRenderInfo& info);
};

#endif // !_ASSETLOADWINDOW_H_
