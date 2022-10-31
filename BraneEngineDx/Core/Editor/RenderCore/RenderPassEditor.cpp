#include "RenderPassEditor.h"
#include "../../GUI/GUIUtility.h"
#include "../../GUI/TextureViewer.h"

RegistEditor(RenderPass);

void RenderPassEditor::setInspectedObject(void* object)
{
	renderPass = static_cast<RenderPass*>(object);
}

void RenderPassEditor::onRenderPassGUI(EditorInfo& info)
{
	vector<pair<string, Texture*>> textures;
	renderPass->getOutputTextures(textures);
	for (int i = 0; i < textures.size(); i++) {
		auto& tex = textures[i];
		ImGui::PushID(i);
		ImGui::BeginHeaderBox(tex.first.c_str());
		Texture2D* tex2D = dynamic_cast<Texture2D*>(tex.second);
		if (tex2D) {
			float width = ImGui::GetWindowContentRegionWidth();
			float aspect = tex2D->getHeight() / (float)tex2D->getWidth();
			tex2D->bind();
			if (ImGui::ImageButton((ImTextureID)tex2D->getTextureID(), { width, width * aspect })) {
				TextureViewer::showTexture(*info.gui, *tex2D);
			}
		}
		ImGui::EndHeaderBox();
		ImGui::PopID();
	}
}

void RenderPassEditor::onGUI(EditorInfo& info)
{
	if (renderPass == NULL)
		return;
	onRenderPassGUI(info);
}
