#include "HelpUI.h"
#include "../../resource.h"

HelpUI::HelpUI() : WUIControl(NULL), titleLabel("Help"), closeButton("¨w")
{
	winStyle = WS_POPUP | WS_SYSMENU;
	setSize({ 250, 245 });
	closeButton.setNormalColor({ 0, 0, 0, 0 });
	closeButton.setPosAndSize({ 220, 0 }, { 30, 30 });
	closeButton.onClickedEvent += [](WUIButton& self) {
		((HelpUI*)self.parent)->hide();
	};
	titleLabel.setPosAndSize({ 20, 5 }, { 210, 20 });
	titleLabel.setTextFlag(DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	textLabel.setText(
		"1. use W and S to control accelerator and brake.\n\n"
		"2. use A and D to control steer.\n\n"
		"3. use Q to switch Vehicle and CubeVehicle.\n\n"
		"4. use SPACE to switch FP and TP camera."
		);
	textLabel.setPosAndSize({ 20, 45 }, { 210, 180 });
	textLabel.setTextFlag(DT_LEFT | DT_EDITCONTROL | DT_WORDBREAK);
	addControl(closeButton);
	addControl(titleLabel);
	addControl(textLabel);
}

BOOL HelpUI::onEraseBkgnd(HDC hdc, const PAINTSTRUCT& ps)
{
	WUIControl::onEraseBkgnd(hdc, ps);
	HBRUSH hbr = CreateSolidBrush(RGB(51, 51, 51));
	RECT rect = { 10, 35, 240, 235 };
	FillRect(hdc, &rect, hbr);
	DrawIcon(hdc, 0, 0, wndClassEx.hIcon);
	return 1;
}
