#pragma once

#include "Spine2DMeshData.h"
#include "Spine2DModel.h"
#include "../Core/Render.h"

class ENGINE_API Spine2DRender : public Render
{
public:
    Spine2DRender();
    virtual ~Spine2DRender();

    void setModel(Spine2DModel* model, spine::Skeleton* skeleton);

    void setSize(const Vector2f& size);
    void setRefSize(float refLength, bool byWidth = true);

    bool get3DMode() const;
    void set3DMode(bool is3DMode);

    Color getOverColor() const;
    void setOverColor(Color color);

    bool loadDefaultMaterial();

    Texture2D* getOutputTexture();

    virtual void preRender();
    virtual void render(RenderInfo& info);
    virtual Matrix4f getTransformMatrix() const;
    virtual IRendering::RenderType getRenderType() const;
    virtual Shape* getShape() const;
    virtual Material* getMaterial(unsigned int index = 0);
    virtual bool getMaterialEnable(unsigned int index = 0);
    virtual Shader* getShader() const;
protected:
    Spine2DMeshData meshData;

    Spine2DModel* modelAsset = NULL;
    spine::Skeleton* skeleton = NULL;

    bool is3DMode = false;
    bool isCulling = false;
    Vector2f viewSize = { 512, 512 };
    Color overColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    BoundBox initBounds;

    Material* material = NULL;

    Texture2D drawTexture = Texture2D(512, 512, 4, false, { TW_Repeat, TW_Repeat, TF_Linear, TF_Linear });
    Texture2D maskTexture = Texture2D(512, 512, 4, false, { TW_Repeat, TW_Repeat, TF_Linear, TF_Linear, TIT_Default, { 1.0f, 1.0f, 1.0f, 1.0f } });
    RenderTarget drawRenderTarget = RenderTarget(512, 512, 4);
    RenderTarget maskRenderTarget = RenderTarget(512, 512, 4);

    void release();
};