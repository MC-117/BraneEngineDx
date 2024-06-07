#pragma once

#include "BaseEditor.h"
#include "../Object.h"

class ObjectBehaviorEditor : public BaseEditor
{
public:
	ObjectBehaviorEditor() = default;
	virtual ~ObjectBehaviorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onBehaviorGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	ObjectBehavior* behavior = NULL;
};

class ObjectEditor : public BaseEditor
{
public:
	ObjectEditor() = default;
	virtual ~ObjectEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onHeaderGUI(EditorInfo& info);
	virtual void onOperationGUI(EditorInfo& info);
	virtual void onBehaviorGUI(EditorInfo& info);
	virtual void onEventGUI(EditorInfo& info);
	virtual void onDetailGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	Object* object = NULL;

	string objectFilterName;
	string duplicateName;
	const Serialization* behaviorType = NULL;
	vector<Object*> objects;
	vector<int> ints;
	vector<float> floats;
	vector<string> strings;
};