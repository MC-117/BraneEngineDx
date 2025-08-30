#include "TextureViewer.h"
#include "../Engine.h"
#include "../TextureCube.h"

TextureViewer::TextureViewer(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void TextureViewer::setTexture(Texture & tex)
{
	texture = &tex;
}

void TextureViewer::onWindowGUI(GUIRenderInfo & info)
{
	float offColor = 0.2f, onColor = 0.7f;
	if (texture != NULL) {
		ImGui::PushStyleColor(ImGuiCol_Button, {channelMask & 0xFF000000 ? onColor : offColor, 0.0f, 0.0f, 1.0f});
		if (ImGui::Button("R")) {
			channelMask ^= 0xFF000000;
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();
		
		ImGui::PushStyleColor(ImGuiCol_Button, {0.0f, channelMask & 0xFF0000 ? onColor : offColor, 0.0f, 1.0f});
		if (ImGui::Button("G")) {
			channelMask ^= 0xFF0000;
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();
		
		ImGui::PushStyleColor(ImGuiCol_Button, {0.0f, 0.0f, channelMask & 0xFF00 ? onColor : offColor, 1.0f});
		if (ImGui::Button("B")) {
			channelMask ^= 0xFF00;
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();

		float alphaCol = channelMask & 0xFF ? onColor : offColor;
		ImGui::PushStyleColor(ImGuiCol_Button, {alphaCol, alphaCol, alphaCol, 1.0f});
		if (ImGui::Button("A")) {
			channelMask ^= 0xFF;
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();
		
		ImGui::Checkbox("Invert", &invert);
		ImGui::SameLine();
		if (ImGui::Button("CaptureToDisk")) {
			Texture2D* texture2D = dynamic_cast<Texture2D*>(texture);
			if (texture2D) {
				FileDlgDesc desc;
				desc.title = "Capture";
				desc.filter = "png(*.png)|*.png|jpg(*.jpg)|*.jpg|tga(*.tga)|*.tga|bmp(*.bmp)|*.bmp";
				desc.initDir = "";
				desc.defFileExt = "png";
				desc.save = true;

				if (openFileDlg(desc)) {
					texture2D->save(desc.filePath);
				}
			}
			TextureCube* textureCube = dynamic_cast<TextureCube*>(texture);
			if (textureCube) {
				FileDlgDesc desc;
				desc.title = "Capture";
				desc.filter = "mip(*.mip)|*.mip";
				desc.initDir = "";
				desc.defFileExt = "png";
				desc.save = true;

				if (openFileDlg(desc)) {
					textureCube->save(desc.filePath);
				}
			}
		}
		string path = AssetInfo::getPath(texture);
		if (!path.empty()) {
			ImGui::SameLine();
			if (ImGui::Button("ReloadFromSource")) {
				Texture2D* texture2D = dynamic_cast<Texture2D*>(texture);
				if (texture2D) {
					texture2D->load(path);
				}
			}
		}
		ImGui::SliderFloat("MipLevel", &mipLevel, 0, texture->getMipLevels());

		float padding = 10;
		ImVec2 pos = ImGui::GetWindowPos();
		float cursorPosY = ImGui::GetCursorPosY();
		pos.y += cursorPosY;
		float width = ImGui::GetWindowWidth() - padding * 2;
		float height = ImGui::GetWindowHeight() - cursorPosY - padding * 2;
		float tw = texture->getWidth();
		float th = texture->getHeight();
		float aspect = tw / th;
		if (aspect < width / height) {
			tw = height * aspect;
			th = height;
		}
		else {
			tw = width;
			th = width / aspect;
		}
		float hpw = (width - tw) / 2.0f;
		float hph = (height - th) / 2.0f;

		auto list = ImGui::GetWindowDrawList();
		ImTextureID id;
		id.ptr = (void*)texture->getTextureID();
		id.mipLevel = mipLevel;
		id.channelMask = channelMask;
		ImVec2 uv_min, uv_max;
		if (invert) {
			uv_min = { 0, 1 };
			uv_max = { 1, 0 };
		}
		else {
			uv_min = { 0, 0 };
			uv_max = { 1, 1 };
		}
		if (id.ptr != 0)
			list->AddImage(id, { pos.x + padding + hpw, pos.y + padding + hph },
				{ pos.x + padding + hpw + tw, pos.y + padding + hph + th }, uv_min, uv_max);
	}
}

void TextureViewer::showTexture(GUI& gui, Texture & tex)
{
	TextureViewer *win = dynamic_cast<TextureViewer*>(gui.getUIControl("TextureViewer"));
	if (win == NULL) {
		win = new TextureViewer();
		gui.addUIControl(*win);
	}
	win->show = true;
	win->setTexture(tex);
}
