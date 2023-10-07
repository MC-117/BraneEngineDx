#pragma once
#include "../Unit.h"
#include "../InitializationManager.h"

class ENGINE_API IProfiler
{
public:
	IProfiler();

	virtual bool init() = 0;
	virtual bool release() = 0;

	virtual bool isValid() const = 0;

	virtual void tick() = 0;

	virtual bool setCapture() = 0;
	virtual bool setNextCapture() = 0;

	virtual void beginScope(const string& name) = 0;
	virtual void endScope() = 0;

	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
};


class ENGINE_API ProfilerManager : public Initialization
{
	friend class IProfiler;
public:
	static ProfilerManager& instance();

	virtual void tick();

	virtual void setCapture();
	virtual void setNextCapture();

	virtual void beginScope(const string& name);
	virtual void endScope();

	virtual void beginFrame();
	virtual void endFrame();
protected:
	unordered_set<IProfiler*> profilors;

	ProfilerManager();
	virtual ~ProfilerManager();

	virtual bool initialize();
	virtual bool finalize();

	void registProfilor(IProfiler& profilor);
};

#define RegistProfiler(ProfilerClass) ProfilerClass ProfilerClass##Instance