#pragma once

#include "Editor.h"

class BaseEditor : public Editor
{
public:
	BaseEditor() = default;
	virtual ~BaseEditor();

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInstanceGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	Base* base = NULL;
};