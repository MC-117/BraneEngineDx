#pragma once

#include "ObjectGraph.h"
#include "../Editor/ObjectEditor.h"

class ObjectGraphEditor : public ObjectBehaviorEditor
{
public:
	ObjectGraphEditor() = default;
	virtual ~ObjectGraphEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onBehaviorGUI(EditorInfo & info);
protected:
	ObjectGraph* objectGraph = NULL;
};