#include "TextureCubeEditorWindow.h"
#include "../Engine.h"

TextureCubeEditorWindow::TextureCubeEditorWindow(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void TextureCubeEditorWindow::onRenderWindow(GUIRenderInfo& info)
{
}
