#include "WUIProgressBar.h"

WUIProgressBar::WUIProgressBar() : WUIControl(NULL)
{
	backColor = { 0.35f, 0.35f, 0.35f };
	winStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD;
}

void WUIProgressBar::setProgress(float progress)
{
	progress = clamp(progress, 0.0f, 1.0f);
	if (this->progress != progress) {
		this->progress = progress;
		updateProgress();
	}
}

float WUIProgressBar::getProgress() const
{
	return progress;
}

void WUIProgressBar::updateProgress()
{
	InvalidateRect(hWnd, NULL, TRUE);
	//SendMessage(hWnd, WM_PAINT, 0, 0);
}

void WUIProgressBar::onPaint(HDC hdc)
{
	RECT crect;
	GetClientRect(hWnd, &crect);

	RECT prect = crect;
	prect.right = crect.left + (crect.right - crect.left) * progress;

	HBRUSH hbr = CreateSolidBrush(RGB(backColor.r * 255, backColor.g * 255, backColor.b * 255));
	FillRect(hdc, &crect, hbr);
	DeleteObject(hbr);

	hbr = CreateSolidBrush(RGB(progressColor.r * 255, progressColor.g * 255, progressColor.b * 255));
	FillRect(hdc, &prect, hbr);
	DeleteObject(hbr);
}
