#pragma once
#include "../Unit.h"
#include "../InitializationManager.h"

class IProfilor
{
public:
	IProfilor();

	virtual bool init() = 0;
	virtual bool release() = 0;

	virtual bool isValid() const = 0;

	virtual void tick() = 0;

	virtual bool setCapture() = 0;

	virtual void beginScope(const string& name) = 0;
	virtual void endScope() = 0;

	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
};


class ProfilorManager : public Initialization
{
	friend class IProfilor;
public:
	static ProfilorManager& instance();

	virtual void tick();

	virtual void setCapture();

	virtual void beginScope(const string& name);
	virtual void endScope();

	virtual void beginFrame();
	virtual void endFrame();
protected:
	unordered_set<IProfilor*> profilors;

	ProfilorManager();
	virtual ~ProfilorManager();

	virtual bool initialze();

	void registProfilor(IProfilor& profilor);
};

#define RegistProfilor(ProfilorClass) ProfilorClass ProfilorClass##Instance