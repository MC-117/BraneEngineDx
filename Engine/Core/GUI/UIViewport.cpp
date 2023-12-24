#include "UIViewport.h"

#include "ImGuiIconHelp.h"
#include "../Engine.h"

UIViewport::UIViewport(const string& name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
	, surface(name)
{
}

void UIViewport::setCamera(Camera* camera)
{
	surface.bindCamera(camera);
}

void UIViewport::onWindowGUI(GUIRenderInfo& info)
{
	ImVec2 imSize = ImGui::GetWindowSize();
	resize({ (int)imSize.x, (int)imSize.y });

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	GUISurface& targetSurface = getTargetSurface();
	Camera* targetCamera = targetSurface.getCamera();
	if (targetCamera == NULL)
		return;
	Texture* sceneTexture = targetCamera->cameraRender.getSceneTexture();
	if (sceneTexture) {
		ImVec2 winPos = ImGui::GetWindowPos();
		drawList->AddImage(ImTextureID(sceneTexture->getTextureID()),
			winPos, { winPos.x + imSize.x, winPos.y + imSize.y });

		if (ImGui::Button(gizmoEnable ? "Gizmo " ICON_FA_EYE : "Gizmo " ICON_FA_EYE_SLASH)) {
			gizmoEnable = !gizmoEnable;
		}

		if (gizmoEnable)
			targetSurface.gizmoFrame(drawList, targetCamera->getRoot());
	}
}

void UIViewport::onPostAction(GUIPostInfo& info)
{
	UIWindow::onPostAction(info);
	GUISurface& targetSurface = getTargetSurface();
	if (show && targetSurface.isValid())
		targetSurface.gizmoRender2D();
}

void UIViewport::onRender(RenderInfo& info)
{
	UIWindow::onRender(info);
	GUISurface& targetSurface = getTargetSurface();
	if (targetSurface.isValid())
		targetSurface.gizmoRender3D(info);
}

void UIViewport::resize(const Vector2i& size)
{
	getTargetSurface().setSize(size);
}

GUISurface& UIViewport::getTargetSurface()
{
	return surface.isValid() ? surface : GUISurface::getMainViewportGUISurface();
}
