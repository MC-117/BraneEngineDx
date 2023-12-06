#include "UIControl.h"
#include "../World.h"

UIControl::UIControl(Object& object, string name, bool defaultShow)
	: object(object), events(this)
{
	this->name = name;
	this->show = defaultShow;
}

UIControl::~UIControl()
{
}

void UIControl::onPreAction(GUIRenderInfo& info)
{
}

void UIControl::onGUI(GUIRenderInfo& info)
{
}

void UIControl::onPostAction(GUIPostInfo& info)
{
}

void UIControl::onSceneResize(const Vector2i& size)
{
}

void UIControl::onAttech(GUI& gui)
{
}

void UIControl::onRender(RenderInfo& info)
{
}
