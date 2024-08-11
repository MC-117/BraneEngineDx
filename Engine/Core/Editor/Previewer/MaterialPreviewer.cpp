#include "MaterialPreviewer.h"

#include "../../Asset.h"

MaterialPreviewer::MaterialPreviewer(const string& name)
    : meshActor("PreviewMeshActor")
    , editorWorld(name + "PreviewWorld")
{
}

void MaterialPreviewer::init(Material* material)
{
    Mesh* mesh = getAssetByPath<Mesh>("Engine/Shapes/UnitSphere.fbx");
    meshActor.setMesh(mesh);
    if (material)
        meshActor.meshRender.setMaterial(0, *material);
    meshActor.setScale({ 10, 10, 10 });
    editorWorld.camera.distance = 15;
    editorWorld.addChild(meshActor);
    editorWorld.camera.clearColor = Color(88, 88, 88, 255);
    editorWorld.getGizmo().setEnableGUI(false);
    editorWorld.getGizmo().setCameraControl(Gizmo::CameraControlMode::Turn, 0, 1, 100);
    //editorWorld.camera.renderTarget.setMultisampleLevel(4);
    editorWorld.begin();
}

void MaterialPreviewer::setMesh(Mesh* mesh)
{
}

void MaterialPreviewer::setMaterial(Material* material)
{
    this->material = material;
    meshActor.meshRender.setMaterial(0, *material);
}

void MaterialPreviewer::setMaterials(vector<Material*> material)
{
    if (!material.empty())
        setMaterial(material.front());
}

void MaterialPreviewer::onGUI(const char* name, int width, int height)
{
    ImGui::BeginChild(name, ImVec2((float)width, (float)height));

    if (material) {
        editorWorld.setViewportSize(width, height);
        editorWorld.update();
        auto drawList = ImGui::GetWindowDrawList();
        GUISurface& guiSurface = editorWorld.getGUISurface();
        Texture* texture = guiSurface.getSceneTexture();
        if (texture != NULL) {
            texture->bind();
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });

            auto list = ImGui::GetWindowDrawList();
            float padding = 0;
            ImVec2 pos = ImGui::GetWindowPos();
            unsigned long long id = texture->getTextureID();
            if (id != 0)
                list->AddImage((ImTextureID)id, { pos.x + padding, pos.y + padding },
                    { pos.x + padding + width, pos.y + padding + height });
            ImGui::PopStyleVar(2);
        }
        editorWorld.onGUI(drawList);
        needRender = true;
    }
    else {
        ImGui::BeginVertical("Vertical", { -1, -1 }, 0.5);
        ImGui::BeginHorizontal("Horizontal", { -1, -1 }, 0.5);
        ImGui::Text("Invalid Material");
        ImGui::EndHorizontal();
        ImGui::EndVertical();
    }
    
    ImGui::EndChild();
}

void MaterialPreviewer::onRender(RenderInfo& info)
{
    if (needRender) {
        editorWorld.render(*info.renderGraph);
        needRender = false;
    }
}
