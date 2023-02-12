#pragma once

#include "ProfileCore.h"

class RenderDocProfiler : public IProfiler
{
public:
	RenderDocProfiler();

	virtual bool init();
	virtual bool release();

	virtual bool isValid() const;

	virtual void tick();

	virtual bool setCapture();

	virtual void beginFrame();
	virtual void endFrame();

	virtual void beginScope(const string& name);
	virtual void endScope();
protected:
	bool enable;
	bool doCapture = false;
	void* deviceHandle = NULL;
	void* windowHandle = NULL;

	string getNewestCapture();
	void startRenderDoc(string capturePath);
};