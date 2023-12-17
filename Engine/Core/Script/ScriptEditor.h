#pragma once

#include "../Editor/BaseEditor.h"
#include "PythonScript.h"
#include "../../ThirdParty/ImGui/imgui_TextEditor.h"

class ScriptBaseEditor : public BaseEditor
{
public:
	ScriptBaseEditor() = default;
	virtual ~ScriptBaseEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	TextEditor& getTextEditor();

	void syncCode();

	bool saveCode();
	bool applyCode();

	virtual void onMenu(EditorInfo& info);
	virtual void onStatusGUI(EditorInfo& info);
	virtual void onTextEditor(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	ScriptBase* scriptBase = NULL;
	int savedUndoIndex = 0;
	TextEditor textEditor;

	virtual void onApplyCode();
};
