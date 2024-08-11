#pragma once

#include "../../Config.h"

class Mesh;
class Material;
struct RenderInfo;

class IEditorPreviewer
{
public:
    virtual void setMesh(Mesh* mesh) = 0;
    virtual void setMaterial(Material* material) = 0;
    virtual void setMaterials(vector<Material*> material) = 0;

    virtual void onGUI(const char* name, int width, int height) = 0;
    virtual void onRender(RenderInfo& info) = 0;
};
