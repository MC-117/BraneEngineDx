#pragma once

#include "EditorPreviewer.h"
#include "../EditorWorld.h"
#include "../../MeshActor.h"

class MaterialPreviewer : public IEditorPreviewer
{
public:
    MaterialPreviewer(const string& name);
    
    void init(Material* material);

    virtual void setMesh(Mesh* mesh);
    virtual void setMaterial(Material* material);
    virtual void setMaterials(vector<Material*> material);

    virtual void onGUI(const char* name, int width, int height);
    virtual void onRender(RenderInfo& info);
protected:
    Material* material = NULL;
    bool needRender = false;
    EditorWorld editorWorld;
    MeshActor meshActor;
};
