#include "PortalActorEditor.h"

#include "PortalManager.h"
#include "Core/ObjectUltility.h"
#include "Core/GUI/GizmoUltility.h"
#include "Core/GUI/GUIUtility.h"
#include "Core/RenderCore/CameraRenderData.h"

RegistEditor(PortalActor);

void PortalActorEditor::setInspectedObject(void* object)
{
    portalActor = dynamic_cast<PortalActor*>((Object*)object);
    ActorEditor::setInspectedObject(portalActor);
}

uint32_t MurmurMix(uint32_t Hash)
{
    Hash ^= Hash >> 16;
    Hash *= 0x85ebca6b;
    Hash ^= Hash >> 13;
    Hash *= 0xc2b2ae35;
    Hash ^= Hash >> 16;
    return Hash;
}

Color IntToColor(uint32_t Index)
{
    uint32_t Hash = MurmurMix(Index);

    return Color
    {
        int((Hash >>  0) & 255),
        int((Hash >>  8) & 255),
        int((Hash >> 16) & 255)
    };
}

void PortalActorEditor::onPersistentGizmo(GizmoInfo& info)
{
    ActorEditor::onPersistentGizmo(info);

    auto drawPortal = [&info] (const PortalActor* portalActor, const Color& color)
    {
        Mesh* mesh = portalActor->portalMeshRender.getMesh();
        if (mesh == NULL)
            return;
        info.gizmo->drawAABB(mesh->bound,
            portalActor->portalMeshRender.transformMat, color);
        PortalManager* portalManager = getObjectBehavior<PortalManager>(*portalActor->getRoot());
        if (portalManager) {
            const CameraRenderData* cameraRenderData = portalManager->getPortalRenderData(portalActor->portalMeshRender.getPortalID());
            if (cameraRenderData)
                drawFrustum(*info.gizmo, cameraRenderData->data, color);
        }
    };

    
    drawPortal(portalActor, IntToColor(portalActor->portalMeshRender.getPortalID()));

    // const PortalActor* connectedPortal = portalActor->connectedPortal;
    // if (connectedPortal) {
    //     drawPortal(connectedPortal, Color(0.0f, 1.0f, 1.0f));
    // }
}

void PortalActorEditor::onHandleGizmo(GizmoInfo& info)
{
    ActorEditor::onHandleGizmo(info);
}

void PortalActorEditor::onActorGUI(EditorInfo& info)
{
    ActorEditor::onActorGUI(info);
    Object* connectedPortal = portalActor->connectedPortal;
    if (ImGui::ObjectCombo("Connected Portal", connectedPortal,
        portalActor->getRoot(), "PortalActor")) {
        portalActor->connect((PortalActor*)connectedPortal);
    }
}

void PortalActorEditor::onRenderersGUI(EditorInfo& info)
{
    ImGui::Text("PortalID: %d", (uint32_t)portalActor->portalMeshRender.getPortalID());
}
