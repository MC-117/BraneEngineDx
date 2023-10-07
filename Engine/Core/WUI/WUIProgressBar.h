#pragma once

#include "WUIFont.h"
#include "../Delegate.h"

class ENGINE_API WUIProgressBar : public WUIControl
{
public:
	WUIProgressBar();

	void setProgress(float progress);
	float getProgress() const;

protected:
	float progress = 0;
	Color progressColor = { 0.00f, 0.68f, 0.84f };

	void updateProgress();

	virtual void onPaint(HDC hdc);
};