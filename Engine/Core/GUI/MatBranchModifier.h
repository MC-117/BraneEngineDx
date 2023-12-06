#pragma once
#ifndef _MATBRANCHMODIFIER_H_
#define _MATBRANCHMODIFIER_H_

#include "UIWindow.h"
#include "../MeshRender.h"
#include "../SkeletonMeshActor.h"

class ENGINE_API MatBranchModifier : public UIWindow
{
public:
	MeshRender* meshRender = NULL;

	MatBranchModifier(string name = "MatBranchModifier", bool defaultShow = false);

	virtual void onWindowGUI(GUIRenderInfo& info);
protected:
	Ref<Object> targetObject;
	string objectFilterName;
	string colorName;
	string scalarName;
	Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	float scalar;
};

#endif // !_MATBRANCHMODIFIER_H_
