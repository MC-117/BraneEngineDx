#include "UIViewport.h"
#include "../Engine.h"

UIViewport::UIViewport(const string& name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void UIViewport::setCamera(Camera* camera)
{
	size.x() = camera->size.x;
	size.y() = camera->size.y;
}

void UIViewport::onRenderWindow(GUIRenderInfo& info)
{
	ImVec2 imSize = ImGui::GetWindowSize();
	Vector2i currentSize(imSize.x, imSize.y);
	if (currentSize != size) {
		resize(currentSize.x(), currentSize.y());
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	Texture* sceneMap = camera->cameraRender.getSceneMap();
	if (sceneMap) {
		ImVec2 winPos = ImGui::GetWindowPos();
		drawList->AddImage(ImTextureID(sceneMap->getTextureID()),
			winPos, { winPos.x + imSize.x, winPos.y + imSize.y });
	}
}

void UIViewport::resize(int width, int height)
{
	if (camera) {
		camera->setSize({ width, height });
		size = { width, height };
	}
}
