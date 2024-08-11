#pragma once

#include "TextureNode.h"
#include "../../Graph/VariableEditor.h"

class TextureParameterVariableEditor : public GraphVariableEditor
{
public:
    TextureParameterVariableEditor() = default;
    virtual ~TextureParameterVariableEditor() = default;

    EditorInfoMethod();
    virtual void setInspectedObject(void* object);

    virtual void onInspectGUI(EditorInfo & info);
protected:
    TextureParameterVariable* textureVariable = NULL;
};
