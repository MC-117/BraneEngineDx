#pragma once

#include "../Editor/Editor.h"
#include "TerrainGeometry.h"

class TerrainGeometryEditor : public Editor
{
public:
	TerrainGeometryEditor() = default;
	virtual ~TerrainGeometryEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGeometryGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	TerrainGeometry* geometry = NULL;
};