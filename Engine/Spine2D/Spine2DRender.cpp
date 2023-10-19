#include "Spine2DRender.h"
#include "../Core/Camera.h"
#include "Spine2DRenderPack.h"

Spine2DRender::Spine2DRender()
{
    drawTexture.setAutoGenMip(false);
    maskTexture.setAutoGenMip(false);
    drawRenderTarget.addTexture("drawTexture", drawTexture);
    maskRenderTarget.addTexture("maskTexture", maskTexture);
}

Spine2DRender::~Spine2DRender()
{
    release();
}

void Spine2DRender::setModel(Spine2DModel* model, spine::Skeleton* skeleton)
{
    if (!loadDefaultMaterial())
        return;

    if (model == NULL || !model->isValid() || skeleton == NULL)
        return;

    release();

    modelAsset = model;
    this->skeleton = skeleton;

    meshData.setModel(skeleton);

    initBounds = meshData.bounds;

    Vector3f size = initBounds.getSize();
    setSize({ size.x(), size.y() });
}

void Spine2DRender::setSize(const Vector2f& size)
{
    if (size.x() * size.y() == 0)
        return;
    viewSize = size;
}

void Spine2DRender::setRefSize(float refLength, bool byWidth)
{
    if (modelAsset == NULL)
        return;

    Vector3f size = initBounds.getSize();
    if (size.x() * size.y() == 0)
        return;
    float aspect = size.x() / size.y();
    float width;
    float height;
    if (byWidth) {
        width = refLength;
        height = refLength / aspect;
    }
    else {
        width = refLength * aspect;
        height = refLength;
    }
    setSize({ width, height });
}

bool Spine2DRender::get3DMode() const
{
    return is3DMode;
}

void Spine2DRender::set3DMode(bool is3DMode)
{
    this->is3DMode = is3DMode;
}

Color Spine2DRender::getOverColor() const
{
    return overColor;
}

void Spine2DRender::setOverColor(Color color)
{
    overColor = color;
}

bool Spine2DRender::loadDefaultMaterial()
{
    if (material != NULL)
        return true;
    material = getAssetByPath<Material>("Engine/Shaders/Spine2D.mat");
    return material != NULL;
}

Texture2D* Spine2DRender::getOutputTexture()
{
    return &drawTexture;
}

void Spine2DRender::preRender(PreRenderInfo& info)
{
}

Color toColor(const spine::Color c)
{
    return Color{ c.r, c.g, c.b, c.a };
}

void Spine2DRender::render(RenderInfo& info)
{
    if (modelAsset == NULL || !modelAsset->isValid() || skeleton == NULL || material == NULL)
        return;

    meshData.release();
    meshData.setModel(skeleton);

    float sizeValue = viewSize.x() * viewSize.y();
    if (sizeValue == 0 || isnan(sizeValue) || isinf(sizeValue))
        return;

    const int partCount = skeleton->getDrawOrder().size();

    /*if (is3DMode)
    {
        info.camera->cameraRender.renderTarget.bindFrame();
    }
    else {
        drawRenderTarget.resize(viewSize.x(), viewSize.y());
        drawRenderTarget.clearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        drawRenderTarget.bindFrame();
    }

    vender.setViewport(0, 0, viewSize.x(), viewSize.y());*/

    Matrix4f mvpMat;

    //if (is3DMode) {
        mvpMat = transformMat;
    /*}
    else {
        float aspect = viewSize.y() / viewSize.x();
        Matrix4f projMat = Matrix4f::Identity();
        projMat(0, 0) = aspect;
        float size = viewSize.x();
        Matrix4f viewMat = Camera::viewport(-1 / size, -2 / size, 2 / size, 2 / size, 0, 0);
        mvpMat = projMat * viewMat * transformMat;
    }*/

    material->setMatrix("projectMatrix", MATRIX_UPLOAD_OP(mvpMat));

    Spine2DRenderCommand command;
    command.sceneData = info.sceneData;
    command.material = material;

    for (int i = 0; i < partCount; i++) {
        spine::Slot* slot = skeleton->getDrawOrder()[i];

        int index = slot->getData().getIndex();

        MeshPart* meshPart = meshData.getMeshPart(index);
        spine::Attachment* attachment = slot->getAttachment();
        if (meshPart == NULL || meshPart->vertexCount == 0)
            continue;

        Texture2D* texture = NULL;
        Color color = toColor(skeleton->getColor());
        Color tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

        if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
            spine::RegionAttachment* regionAttachment = (spine::RegionAttachment*)attachment;
            texture = (Texture2D*)((spine::AtlasRegion*)regionAttachment->getRendererObject())->page->getRendererObject();
            tintColor = toColor(regionAttachment->getColor());
        }
        else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
            texture = (Texture2D*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();
            tintColor = toColor(mesh->getColor());
        }
        else {
            continue;
        }

        color *= tintColor * overColor;

        command.mesh = meshPart;
        command.resource.texture = texture;
        command.resource.color = color;
        command.resource.cullType = isCulling ? CullType::Cull_Back : CullType::Cull_Off;

        spine::BlendMode blendMode = slot->getData().getBlendMode();

        switch (blendMode)
        {
        case spine::BlendMode_Normal:
        default:
            command.blendMode = BlendMode::BM_PremultiplyAlpha;
            break;

        case spine::BlendMode_Additive:
            command.blendMode = BlendMode::BM_Additive;
            break;

        case spine::BlendMode_Multiply:
            command.blendMode = BlendMode::BM_Multipy;
            break;
        }

        info.renderGraph->setRenderCommand(command);
    }
}

Matrix4f Spine2DRender::getTransformMatrix() const
{
    return Matrix4f();
}

IRendering::RenderType Spine2DRender::getRenderType() const
{
    return IRendering::Normal_Render;
}

Shape* Spine2DRender::getShape() const
{
    return nullptr;
}

Material* Spine2DRender::getMaterial(unsigned int index)
{
    return nullptr;
}

bool Spine2DRender::getMaterialEnable(unsigned int index)
{
    return false;
}

Shader* Spine2DRender::getShader() const
{
    return nullptr;
}

void Spine2DRender::release()
{
    modelAsset = NULL;
    skeleton = NULL;
    meshData.release();
}
