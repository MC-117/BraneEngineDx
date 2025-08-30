#include "DecalRender.h"

#include "RenderCore/DecalRenderPack.h"
#include "Utility/MathUtility.h"

DecalRender::DecalRender()
{
    canCastShadow = false;
    hasPrePass = false;
}

Enum<DecalMask> DecalRender::getDecalMask() const
{
    return decalMask;
}

void DecalRender::setDecalMask(Enum<DecalMask> mask)
{
    decalMask = mask;
}

void DecalRender::setDecalBoxMesh()
{
    if (Mesh* mesh = getAssetByPath<Mesh>("Engine/Shapes/UnitBox.obj")) {
        setMesh(mesh);
    }
}

void DecalRender::render(RenderInfo& info)
{
    if (decalMask.enumValue == Decal_None) {
        return;
    }

    collection.streamInAssets();

    Matrix4f viewMat = Math::lookAt(Vector3f::Zero(), Vector3f::UnitX(), Vector3f::UnitZ());
    Matrix4f projMat = Math::orthotropic(-0.5f, 0.5f, -0.5f, 0.5f, -0.1, 0.1);
    Matrix4f localToDecalClip = projMat * viewMat;
    
    MeshMaterialCollection::DispatchData dispatchData;
    dispatchData.init<DecalRenderCommand>();
    dispatchData.hidden = hidden;
    dispatchData.isStatic = isStatic;
    dispatchData.canCastShadow = false;
    dispatchData.hasPrePass = false;

    auto bindRenderData = [decalMask = decalMask, localToDecalClip](MeshRenderCommand& command)
    {
        DecalRenderCommand& decalCommand = (DecalRenderCommand&)command;
        decalCommand.decalMask = decalMask;
        decalCommand.localToDecalClip = localToDecalClip;
    };

    dispatchData.renderDelegate += bindRenderData;

    collection.dispatchMeshDraw(dispatchData);
}

bool DecalRender::getCanCastShadow() const
{
    return false;
}

bool DecalRender::deserialize(const SerializationInfo& from)
{
    from.get("decalMask", SEnum(&decalMask));
    outlineCollection.deserialize(from);
    collection.deserialize(from);
    return true;
}

bool DecalRender::serialize(SerializationInfo& to)
{
    to.set("decalMask", (int)decalMask);
    outlineCollection.serialize(to);
    collection.serialize(to);
    return true;
}
