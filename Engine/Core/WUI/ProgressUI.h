#pragma once

#include "WUILabel.h"
#include "WUIProgressBar.h"
#include "../Utility/Utility.h"

class ENGINE_API ProgressUI : public WUIControl
{
public:
	WUILabel titleLabel;
	WUILabel progressLabel;
	WUIProgressBar progressBar;
	LongProgressWork work;

	ProgressUI(const string& name);

	void setProgress(float progress);
	float getProgress() const;

	void setCenter();

	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
};