#pragma once

#include "Editor.h"

class MaterialEditor : public Editor
{
public:
	MaterialEditor() = default;
	virtual ~MaterialEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onMaterialGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	Material* material = NULL;
};