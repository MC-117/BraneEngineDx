#pragma once

#include "MeshRender.h"

class ENGINE_API InstancedMeshRender : public Render
{
public:
    bool frustumCulling = true;
    bool hasPrePass = false;

    InstancedMeshRender();

    bool getNeedUpdate() const;
    void setNeedUpdate(bool update);
	
    int getMeshCount() const;
    MeshMaterialCollection* getMesh(int index) const;

    MeshMaterialCollection* addMesh(Mesh& mesh);

    MeshBatchDrawData& getMeshBatchDrawData();

    virtual void render(RenderInfo& info);
    virtual Matrix4f getTransformMatrix() const;
    virtual IRendering::RenderType getRenderType() const;
    virtual Shape* getShape() const;
    virtual Material* getMaterial(unsigned int index = 0);
    virtual bool getMaterialEnable(unsigned int index = 0);
    virtual Shader* getShader() const;
    virtual InstancedTransformRenderDataHandle getInstancedTransformRenderDataHandle(unsigned int index) const;
protected:
    vector<MeshMaterialCollection*> collections;
    MeshTransformRenderData meshTransformRenderData;
    MeshBatchDrawCommandArray meshBatchDrawCommandArray;
    MeshBatchDrawData meshBatchDrawData;
};
