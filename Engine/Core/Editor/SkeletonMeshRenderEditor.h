#pragma once

#include "MeshRenderEditor.h"
#include "../SkeletonMeshRender.h"

class SkeletonMeshRenderEditor : public MeshRenderEditor
{
public:
	SkeletonMeshRenderEditor() = default;
	virtual ~SkeletonMeshRenderEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onRendererGUI(EditorInfo& info);
protected:
	SkeletonMeshRender* skeletonMeshRender = NULL;
};