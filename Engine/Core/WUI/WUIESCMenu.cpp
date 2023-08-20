#include "WUIESCMenu.h"
#include "../Engine.h"
#include "../Console.h"
#include "../GUI/EditorWindow.h"

WUIESCMenu::WUIESCMenu()
	: WUIControl(NULL), menuLabel("Menu"), helpButton("Help"),
	playButton("Play"), fullScreenButton(), quitButton("Quit")
	, editorButton("ShowEditor")
{
	winStyle = WS_POPUP | WS_SYSMENU;
	setSize({ 130, 265 });
	menuLabel.setPosAndSize({ 10, 5 }, { 110, 20 });
	menuLabel.setTextFlag(DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	playButton.setPosAndSize({ 10, 35 }, { 110, 40 });
	helpButton.setPosAndSize({ 10, 80 }, { 110, 40 });
	fullScreenButton.setPosAndSize({ 10, 125 }, { 110, 40 });
	quitButton.setPosAndSize({ 10, 170 }, { 110, 40 });
	playButton.onClickedEvent += [](WUIButton& self) {
		((WUIESCMenu*)self.parent)->play();
	};
	helpButton.onClickedEvent += [](WUIButton& self) {
		((WUIESCMenu*)self.parent)->help();
	};
	fullScreenButton.onClickedEvent += [](WUIButton& self) {
		((WUIESCMenu*)self.parent)->toggleFullscreen();
	};
	quitButton.onClickedEvent += [](WUIButton& self) {
		((WUIESCMenu*)self.parent)->quit();
	};
	addControl(menuLabel);
	addControl(playButton);
	addControl(helpButton);
	addControl(fullScreenButton);
	addControl(quitButton);

	editorButton.setPosAndSize({ 10, 215 }, { 110, 40 });
	editorButton.onClickedEvent += [](WUIButton& self) {
		((WUIESCMenu*)self.parent)->editor();
	};
	addControl(editorButton);
}

void WUIESCMenu::play()
{
	hide();
	World* world = Engine::getCurrentWorld();
	if (world == NULL)
		return;
	if (!world->renderPool.gui.isShowUIControl("Editor"))
	Engine::input.setCursorHidden(true);
}

void WUIESCMenu::help()
{
	if (parent == NULL)
		return;
	if (helpUI.parent == NULL) {
		parent->addControl(helpUI);
	}
	helpUI.show();
	helpUI.setPos(parent->getPos() + (parent->getSize() - helpUI.getSize()) / 2);
}

void WUIESCMenu::toggleFullscreen()
{
	Engine::toggleFullscreen();
}

void WUIESCMenu::quit()
{
	World* world = Engine::getCurrentWorld();
	if (world == NULL)
		return;
	world->quit();
}

void WUIESCMenu::editor()
{
	World* world = Engine::getCurrentWorld();
	if (world == NULL)
		return;
	UIControl* uc = world->renderPool.gui.getUIControl("Editor");
	bool b = false;
	if (uc != NULL) {
		EditorWindow* win = dynamic_cast<EditorWindow*>(uc);
		if (win != NULL) {
			win->show = true;
		}
		else
			b = true;
	}
	else
		b = true;
	if (b) {
		EditorWindow* win = new EditorWindow(*world, Material::nullMaterial, "Editor", true);
		(*world) += win;
	}
}

void WUIESCMenu::onParentMove(const Unit2Di& pos, const Unit2Di& size)
{
	setPos(pos + (size - this->size) / 2);
	helpUI.setPos(pos + (size - helpUI.getSize()) / 2);
}

void WUIESCMenu::onParentResize(WPARAM wParam, const Unit2Di& pos, const Unit2Di& size)
{
	setPos(pos + (size - this->size) / 2);
	helpUI.setPos(pos + (size - helpUI.getSize()) / 2);
}

void WUIESCMenu::onLoop()
{
	WUIControl::onLoop();
	fullScreenButton.setText(Engine::windowContext.fullscreen ? "Windowed" : "Fullscreen");
	World* world = Engine::getCurrentWorld();
	if (world == NULL)
		return;
	Input& input = Engine::input;
	if (input.getKeyPress(VK_ESCAPE)) {
		if (isHide()) {
			show(false);
			input.setCursorHidden(false);
		}
		else {
			hide();
#ifdef UI_USE_IMGUI
			if (!world->renderPool.gui.isShowUIControl("Editor"))
#endif // UI_USE_IMGUI
			input.setCursorHidden(true);
			helpUI.hide();
		}
	}
}
