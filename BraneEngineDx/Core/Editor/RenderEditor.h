#pragma once

#include "Editor.h"
#include "../Render.h"

class RenderEditor : public Editor
{
public:
	RenderEditor() = default;
	virtual ~RenderEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onRendererGUI(EditorInfo& info);
	virtual void onShapeGUI(EditorInfo& info);
	virtual void onOutlineGUI(EditorInfo& info);
	virtual void onMaterialsGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	Render* render = NULL;
};