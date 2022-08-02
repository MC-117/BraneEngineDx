#include "TextureViewer.h"
#include "../Engine.h"

TextureViewer::TextureViewer(string name, bool defaultShow)
	: UIWindow(world, name, defaultShow)
{
}

void TextureViewer::setTexture(Texture & tex)
{
	texture = &tex;
}

void TextureViewer::onRenderWindow(GUIRenderInfo & info)
{
	if (texture != NULL) {
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
		unsigned long long id = texture->getTextureID();
		ImVec2 uv_min, uv_max;
		if (invert) {
			uv_min = { 0, 1 };
			uv_max = { 1, 0 };
		}
		else {
			uv_min = { 0, 0 };
			uv_max = { 1, 1 };
		}
		if (id != 0)
			list->AddImage({ id, mipLevel }, { pos.x + padding + hpw, pos.y + padding + hph },
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
