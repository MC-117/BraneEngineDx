#include "TextureNodeEditor.h"
#include "../../GUI/GUIUtility.h"
#include "../../Utility/EngineUtility.h"

RegistEditor(TextureParameterVariable);

void TextureParameterVariableEditor::setInspectedObject(void* object)
{
    textureVariable = dynamic_cast<TextureParameterVariable*>((Base*)object);
    GraphVariableEditor::setInspectedObject(textureVariable);
}

void TextureParameterVariableEditor::onInspectGUI(EditorInfo& info)
{
    Texture* texture = textureVariable->getDefaultValue();
    ImGui::Columns(2, "TextureColumn", false);
    unsigned long long texId = 0;
    if (texture) {
        texId = texture->getTextureID();
    }
    bool click = false;
    if (texId == 0) {
        click = ImGui::Button("##ImgBT", { 64, 64 });
    }
    else {
        click = ImGui::ImageButton((ImTextureID)texId, { 64, 64 }, { 0, 1 }, { 1, 0 });
    }
    if (click) {
        ImGui::OpenPopup("TexSelectPopup");
    }
    ImGui::NextColumn();
    ImGui::Text(textureVariable->getName().c_str());
    string filename = getFileNameWithoutExt(AssetInfo::getPath(texture));
    ImGui::Text(filename.c_str());
    if (ImGui::ArrowButton("AssignTexBT", ImGuiDir_Left)) {
        Asset* selectedAsset = EditorManager::getSelectedAsset();
        if (selectedAsset != NULL && selectedAsset->assetInfo.type == "Texture2D") {
            texture = (Texture*)selectedAsset->load();
        }
    }
    ImGui::NextColumn();
    ImGui::Columns();
    textureVariable->setDefaultValue(texture);
}
