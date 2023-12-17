#pragma once
#include "ScriptEditor.h"
#include "PythonScript.h"

class PythonScriptEditor : public ScriptBaseEditor
{
public:
    PythonScriptEditor() = default;
    virtual ~PythonScriptEditor();

    EditorInfoMethod();
    virtual void setInspectedObject(void* object);

    virtual void onMenu(EditorInfo& info);
    virtual void onTextEditor(EditorInfo& info);
protected:
    PythonScript* pythonScript = NULL;
    PythonRuntimeObject* runtimeObject = NULL;
    vector<PythonFunctionObject> functions;

    virtual void onApplyCode();
};
