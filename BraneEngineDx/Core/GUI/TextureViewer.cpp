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
		float padding = 10;
		ImVec2 pos = ImGui::GetWindowPos();
		float width = ImGui::GetWindowWidth() - padding * 2;
		float height = ImGui::GetWindowHeight() - padding * 2;
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
		if (id != 0)
			list->AddImage((ImTextureID)id, { pos.x + padding + hpw, pos.y + padding + hph }, { pos.x + padding + hpw + tw, pos.y + padding + hph + th }, { 0, 1 }, { 1, 0 });
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
