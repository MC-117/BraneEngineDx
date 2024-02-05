#pragma once
#include "ShaderGraph.h"
#include "../../Graph/GraphEditor.h"

class ShaderGraphEditor : public GraphEditor
{
public:
    ShaderGraphEditor() = default;
    virtual ~ShaderGraphEditor() = default;

    EditorInfoMethod();
    virtual void setInspectedObject(void* object);

    
protected:
    ShaderGraph* shaderGraph = NULL;
    virtual void getPinMenuTags(vector<Name>& tags);
    virtual void getVariableMenuTags(vector<Name>& tags);
    virtual void getNodeMenuTags(vector<Name>& tags);
};
