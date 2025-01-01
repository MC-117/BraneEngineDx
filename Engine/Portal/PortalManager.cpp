#include "PortalManager.h"

#include "../Core/Utility/RenderUtility.h"
#include "../Core/CameraManager.h"
#include "../Core/Utility/MathUtility.h"
#include "../Core/RenderCore/RenderCore.h"
#include "../Core/World.h"
#include "../Core/RenderCore/RenderCoreUtility.h"

struct PortalCameraRenderData : CameraRenderData
{
    CameraRenderData* sourceRenderData = NULL;

    PortalCameraRenderData(CameraRenderData* sourceRenderData) : sourceRenderData(sourceRenderData) {}

    virtual void create()
    {
        // flags = cameraRender->getCameraRenderFlags();

        // renderOrder = cameraRender->renderOrder;
        // sceneTexture = cameraRender->getSceneTexture();
        // surface.renderTarget = &cameraRender->getRenderTarget();

        forceStencilTest = true;

        cullingContext.init();
        staticCullingContext.init();
    }

    virtual void updateSurfaceBuffer(RenderGraph* renderGraph)
    {
        sourceRenderData->updateSurfaceBuffer(renderGraph);
        surfaceBuffer = sourceRenderData->surfaceBuffer;
        surface = sourceRenderData->surface;
        surface.clearFlags = Clear_None;
    }
};

void applyNewTransform(CameraData& data, const Matrix4f& transformMatrix)
{
    data.cameraDir = Math::getForwardVector(transformMatrix);
    data.cameraLeft = Math::getLeftwardVector(transformMatrix);
    data.cameraUp = Math::getUpwardVector(transformMatrix);
    data.cameraLoc = Math::getPosition(transformMatrix);
    data.viewMat = Math::lookAt(data.cameraLoc, data.cameraLoc + data.cameraDir, data.cameraUp);
    data.viewOriginMat = Math::lookAt(Vector3f::Zero(), data.cameraDir, data.cameraUp);
    data.viewMatInv = data.viewMat.inverse();
    data.projectionViewMat = data.projectionMat * data.viewMat;
}

void applyNewTransform(CameraData& data, const Vector3f& position, const Quaternionf& rotation)
{
    data.cameraDir = rotation * Vector3f::UnitX();
    data.cameraLeft = rotation * Vector3f::UnitY();
    data.cameraUp = rotation * Vector3f::UnitZ();
    data.cameraLoc = position;
    data.viewMat = Math::lookAt(data.cameraLoc, data.cameraLoc + data.cameraDir, data.cameraUp);
    data.viewOriginMat = Math::lookAt(Vector3f::Zero(), data.cameraDir, data.cameraUp);
    data.viewMatInv = data.viewMat.inverse();
    data.projectionViewMat = data.projectionMat * data.viewMat;
}

SerializeInstance(PortalManager);

bool PortalManager::init(Object* object)
{
    world = castTo<World>(object);
    if (world)
    {
        registerExistedPortalActor();
        return ObjectBehavior::init(object);
    }
    Console::error("PortalManager only support World");
    return false;
}

void PortalManager::begin()
{
    ObjectBehavior::begin();
}

void PortalManager::tick(float deltaTime)
{
    ObjectBehavior::tick(deltaTime);
}

void PortalManager::afterTick()
{
    ObjectBehavior::afterTick();
}

void PortalManager::prerender(SceneRenderData& sceneData)
{
    ObjectBehavior::prerender(sceneData);

    solvePortalRendering(world->getCameraManager());
}

void PortalManager::end()
{
    ObjectBehavior::end();
}

void PortalManager::registerPortalActor(PortalActor& portal)
{
    portalActors.insert(&portal);
}

void PortalManager::unregisterPortalActor(PortalActor& portal)
{
    for (auto b = portalActors.begin(), e = portalActors.end(); b != e; b++)
    {
        if (*b == &portal)
        {
            portalActors.erase(b);
            return;
        }
    }
}

const CameraRenderData* PortalManager::getPortalRenderData(int portalID) const
{
    --portalID;
    if (portalID >= 0 && portalID < visiablePortalCount) {
        return portalRenderDatas[portalID];
    }
    return NULL;
}

Serializable* PortalManager::instantiate(const SerializationInfo& from)
{
    return new PortalManager();
}

PortalCameraRenderData* PortalManager::fetchPortalRenderData(CameraRenderData* sourceRenderData)
{
    PortalCameraRenderData* portalRenderData = NULL;
    if (visiablePortalCount < portalRenderDatas.size())
    {
        portalRenderData = (PortalCameraRenderData*)portalRenderDatas[visiablePortalCount];
        portalRenderData->sourceRenderData = sourceRenderData;
    }
    else {
        portalRenderData = new PortalCameraRenderData(sourceRenderData);
        portalRenderDatas.emplace_back(portalRenderData);
    }
    visiablePortalCount++;
    return portalRenderData;
}

void PortalManager::solvePortalRendering(ICameraManager& cameraManager)
{
    Camera* camera = cameraManager.getCamera(CameraTag::main);
    // Camera* camera = &world->getDefaultCamera();
    if (camera == NULL)
        return;

    visiablePortalCount = 0;

    struct PortalCameraInfo
    {
        CameraData data;
        Matrix4f transMat;
    };
    
    vector<CameraData> camInput = { camera->cameraRender.cameraData };

    CameraData cameraData = camera->cameraRender.cameraData;
    Matrix4f camTransMat = camera->getTransformMat();

    for (auto& portalActorRef : portalActors) {
        PortalActor* portalActor = portalActorRef;
        portalActor->portalMeshRender.setPortalID(-1);
        portalActor->portalRender.setRenderData(NULL);
    }

    for (auto& portalActorRef : portalActors)
    {
        PortalActor* portalActor = portalActorRef;
        if (portalActor == NULL || portalActor->isHidden())
            continue;
        PortalActor* connectedPortal = portalActor->connectedPortal;
        if (connectedPortal == NULL)
            continue;

        bool faceFront = portalActor->getForward(WORLD).dot(cameraData.cameraDir) < 0;
        if (faceFront && frustumCulling(cameraData, portalActor->portalMeshRender.getMesh()->bound, portalActor->getTransformMat()))
        {
            PortalCameraRenderData* portalRenderData = fetchPortalRenderData(camera->cameraRender.getRenderData());
            const int portalID = visiablePortalCount;
            portalActor->portalMeshRender.setPortalID(portalID);
            portalRenderData->cameraRenderID = portalActor->portalRender.getCameraRenderID();
            portalRenderData->renderOrder = portalID;
            portalRenderData->stencilRef = portalID;
            portalRenderData->data = cameraData;
            portalRenderData->data.clipPlane.setPlane(
                connectedPortal->getForward(WORLD),
                connectedPortal->getPosition(WORLD));

            Matrix4f cameraOffsetMatrix = portalActor->getUnscaledTransformMat().inverse() * camTransMat;
            Vector3f localPosition, unusedScale;
            Quaternionf localRotation;
            cameraOffsetMatrix.decompose(localPosition, localRotation, unusedScale);
            
            // Matrix4f mirroredCameraOffsetMatrix = Math::getTransitionMatrix(localPosition.cwiseProduct({ -1, -1, 1 }))
            //     * Math::getRotationMatrix(localRotation * Quaternionf::FromAngleAxis(Math::PI, Vector3f::UnitZ()));
            // Matrix4f newTransformMatrix = connectedPortal->getUnscaledTransformMat() * mirroredCameraOffsetMatrix;
            //
            // applyNewTransform(portalRenderData->data, newTransformMatrix);

            
            applyNewTransform(portalRenderData->data, connectedPortal->getPosition(WORLD) + localPosition.cwiseProduct({ -1, -1, 1 }),
                connectedPortal->getRotation(WORLD) * (Quaternionf::FromAngleAxis(Math::PI, Vector3f::UnitZ()) * localRotation));
            
            camInput.emplace_back(portalRenderData->data);
            portalActor->portalRender.setRenderData(portalRenderData);
        }
    }
    
    // for (int depth = 0; depth < 1; depth++)
    // {
    //     vector<CameraData> curCamInput = std::move(camInput);
    //
    //     for (auto& cameraData : curCamInput)
    //     {
    //         
    //     }
    //
    //     if (camInput.empty())
    //         break;
    // }
}

void PortalManager::registerExistedPortalActor()
{
    assert(world);
    ObjectIterator iter(world);
    while (iter.next())
    {
        if (PortalActor* portalActor = castTo<PortalActor>(iter.current()))
        {
            registerPortalActor(*portalActor);
        }
    }
}
