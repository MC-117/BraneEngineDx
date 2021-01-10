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

void UIControl::render(GUIRenderInfo& info)
{
}

void UIControl::onPostAction(GUIPostInfo& info)
{
}

void UIControl::onSceneResize(Unit2Di size)
{
}

void UIControl::onAttech(GUI& gui)
{
}
