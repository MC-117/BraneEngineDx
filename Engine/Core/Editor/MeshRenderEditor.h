#pragma once

#include "RenderEditor.h"
#include "../MeshRender.h"

class MeshRenderEditor : public RenderEditor
{
public:
	MeshRenderEditor() = default;
	virtual ~MeshRenderEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onShapeGUI(EditorInfo & info);
	virtual void onOutlineGUI(EditorInfo& info);
	virtual void onMaterialsGUI(EditorInfo & info);
protected:
	MeshRender* meshRender = NULL;
	bool create = false;
};