#include "DecalActorEditor.h"
#include "../Camera.h"
#include "../Engine.h"
#include "../GUI/GizmoUltility.h"
#include "Core/GUI/GUIUtility.h"

RegistEditor(DecalActor);

Texture2D* DecalActorEditor::decalIcon = NULL;

void DecalActorEditor::setInspectedObject(void* object)
{
    decalActor = dynamic_cast<DecalActor*>((Object*)object);
    ActorEditor::setInspectedObject(decalActor);
}

void DecalActorEditor::onPersistentGizmo(GizmoInfo& info)
{
    ActorEditor::onPersistentGizmo(info);

    Texture2D* icon = getIcon();

    Vector3f pos = decalActor->getPosition(WORLD);

    if (icon != NULL) {
        info.gizmo->drawIcon(*icon, pos, getFitIconSize(info));
        if (info.gizmo->pickIcon(pos, Vector2f::Zero(),
            { (float)icon->getWidth(), (float)icon->getHeight() }))
            EditorManager::selectObject(decalActor);
    }
}

void DecalActorEditor::onHandleGizmo(GizmoInfo& info)
{
    ActorEditor::onHandleGizmo(info);
    Mesh* mesh = decalActor->decalRender.getMesh();
    if (mesh == NULL)
        return;
    info.gizmo->drawAABB(mesh->bound,
        decalActor->decalRender.transformMat, Color(0.0f, 1.0f, 0.5f));
}

void DecalActorEditor::onRenderersGUI(EditorInfo& info)
{
    ImGui::BeginGroupPanel("DecalMask");

    int decalMaskValue = decalActor->decalRender.getDecalMask();
    
    ImGui::CheckboxFlags("BaseColor", &decalMaskValue, Decal_BaseColor);
    ImGui::CheckboxFlags("Normal", &decalMaskValue, Decal_Normal);
    ImGui::CheckboxFlags("Roughness", &decalMaskValue, Decal_Roughness);
    ImGui::CheckboxFlags("Metallic", &decalMaskValue, Decal_Metallic);
    ImGui::CheckboxFlags("Specular", &decalMaskValue, Decal_Specular);
    ImGui::CheckboxFlags("AO", &decalMaskValue, Decal_AO);
    ImGui::CheckboxFlags("Emission", &decalMaskValue, Decal_Emission);

    decalActor->decalRender.setDecalMask(decalMaskValue);
    
    ImGui::EndGroupPanel();
    ActorEditor::onRenderersGUI(info);
}

Texture2D* DecalActorEditor::getIcon()
{
    if (decalIcon == NULL)
        decalIcon = getAssetByPath<Texture2D>("Engine/Icons/Decal_Icon.png");
    return decalIcon;
}


