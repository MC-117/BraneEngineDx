#include "ProgressUI.h"
#include "../Engine.h"

ProgressUI::ProgressUI(const string& name) : WUIControl(NULL)
{
	winStyle = WS_POPUP | WS_SYSMENU;
	setSize({ 500, 100 });
	titleLabel.setText(name);
	titleLabel.setPosAndSize({ 10, 5 }, { 480, 20 });
	titleLabel.setTextFlag(DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	progressBar.setPosAndSize({ 10, 30 }, { 450, 20 });
	progressLabel.setText("0%");
	progressLabel.setPosAndSize({ 460, 30 }, { 30, 20 });
	progressLabel.setTextFlag(DT_RIGHT | DT_EDITCONTROL | DT_WORDBREAK);
	addControl(titleLabel);
	addControl(progressBar);
	addControl(progressLabel);
	work.data = this;
	work.callback += [](const LongProgressWork& work) {
		ProgressUI* ui = (ProgressUI*)work.data;
		ui->setProgress(work.progress);
		ui->titleLabel.setText(work.text);
	};
	setCenter();
}

void ProgressUI::setProgress(float progress)
{
	progressBar.setProgress(progress);
	progressLabel.setText(to_string((int)(progressBar.getProgress() * 100)) + '%');
}

float ProgressUI::getProgress() const
{
	return 0.0f;
}

void ProgressUI::setCenter()
{
	RECT rect;
	GetClientRect(Engine::windowContext.hwnd, &rect);
	Unit2Di center = { (rect.left + rect.right) / 2, (rect.bottom + rect.top) / 2 };
	setPos(center - getSize() / 2);
}

LRESULT ProgressUI::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Out = WUIControl::WndProc(msg, wParam, lParam);
	if (msg == WM_NCHITTEST) {
		Out = Out == HTCLIENT ? HTCAPTION : Out;
	}
	return Out;
}
