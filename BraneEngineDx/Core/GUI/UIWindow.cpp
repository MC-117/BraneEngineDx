#include "UIWindow.h"

UIWindow::UIWindow(Object & object, string name, bool defaultShow) : UIControl(object, name, defaultShow)
{
}

UIWindow::~UIWindow()
{
}

void UIWindow::setBackgroundImg(Texture2D & img)
{
	backgroundTex = &img;
}

void UIWindow::setBackgroundColor(Color color)
{
	this->backgroundColor = color;
}

void UIWindow::setStyle(ImGuiWindowFlags style)
{
	this->style = style;
}

bool UIWindow::isFocus() const
{
	return _isFocus;
}

void UIWindow::setFocus()
{
	show = true;
	nextFocus = true;
}

void UIWindow::render(GUIRenderInfo& info)
{
	int styleCount = styleVars.size() + styleFVars.size();
	for (auto b = styleVars.begin(), e = styleVars.end(); b != e; b++)
		ImGui::PushStyleVar(b->first, b->second);
	for (auto b = styleFVars.begin(), e = styleFVars.end(); b != e; b++)
		ImGui::PushStyleVar(b->first, b->second);
	if (ImGui::Begin(name.c_str(), showCloseButton ? &show : NULL, (backgroundTex == NULL || blurBackground) ? style : (style | ImGuiWindowFlags_NoBackground))) {
		if (nextFocus) {
			ImGui::SetWindowFocus();
			nextFocus = false;
		}
		_isFocus = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		if (blurBackground) {
			if (info.sceneBlurTex != NULL) {
				ImGuiStyle& style = ImGui::GetStyle();
				ImVec2 pos = ImGui::GetWindowPos();
				ImVec2 size = ImGui::GetWindowSize();
				ImVec2 b = { pos.x + size.x, pos.y + size.y };
				ImGui::GetBackgroundDrawList()->AddImageRounded((void*)info.sceneBlurTex->getTextureID(), pos, b,
					{ pos.x / (float)info.viewSize.x, pos.y / (float)info.viewSize.y },
					{ b.x / (float)info.viewSize.x, b.y / (float)info.viewSize.y }, (ImColor&)backgroundColor, style.WindowRounding);//, ImDrawCornerFlags_Top);
			}
		}
		onRenderWindow(info);
		ImGui::End();
	}
	else {
		_isFocus = false;
	}
	ImGui::PopStyleVar(styleCount);
}

void UIWindow::onRenderWindow(GUIRenderInfo& info)
{
}
