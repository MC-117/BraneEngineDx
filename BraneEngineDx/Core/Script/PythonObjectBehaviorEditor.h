#pragma once

#include "PythonScript.h"
#include "../Editor/ObjectEditor.h"

class PythonObjectBehaviorEditor : public ObjectBehaviorEditor
{
public:
	PythonObjectBehaviorEditor() = default;
	virtual ~PythonObjectBehaviorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onBehaviorGUI(EditorInfo& info);
protected:
	PythonObjectBehavior* pythonObjectBehavior = NULL;
};