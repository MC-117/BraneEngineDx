#pragma once

#include "Config.h"

class InitializationManager;
class Initialization
{
	friend InitializationManager;
protected:
	int priority = 0;
	Initialization(int priority);
	virtual bool initialze() = 0;
};

class InitializationManager
{
	friend Initialization;
public:
	static InitializationManager& instance();
	void initialze();
protected:
	multimap<int, Initialization*> initializations;

	InitializationManager() = default;
	void addInitialization(Initialization& initialization);
};