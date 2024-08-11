#pragma once
#include "ShaderGraph.h"
#include "../../Graph/GraphEditor.h"

class ShaderGraphEditor : public GraphEditor
{
public:
    ShaderGraphEditor();
    virtual ~ShaderGraphEditor() = default;

    EditorInfoMethod();
    virtual void setInspectedObject(void* object);

    virtual void onDetailsGUI(EditorInfo& info);
protected:
    ShaderGraph* shaderGraph = NULL;
    virtual void getPinMenuTags(vector<Name>& tags);
    virtual void getVariableMenuTags(vector<Name>& tags);
    virtual void getNodeMenuTags(vector<Name>& tags);

    virtual void onGenerateCode(ScriptBase& script);

    virtual void onCompileGUI(EditorInfo& info);

    virtual void onCustomGUI(EditorInfo& info);
};
