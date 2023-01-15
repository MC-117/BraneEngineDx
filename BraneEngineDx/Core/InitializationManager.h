#pragma once

#include "Config.h"

enum InitializeStage : short
{
	BeforeEngineConfig,
	BeforeEngineSetup,
	BeforeWindowSetup,
	BeforeRenderVendorSetup,
	BeforeAssetLoading,
	AfterEngineSetup,
	Num
};

class InitializationManager;
class Initialization
{
	friend InitializationManager;
protected:
	InitializeStage stage = InitializeStage::BeforeAssetLoading;
	int priority = 0;
	Initialization(InitializeStage stage, int priority);
	virtual bool initialze() = 0;
};

class InitializationManager
{
	friend Initialization;
public:
	static InitializationManager& instance();
	void initialze(InitializeStage stage);
protected:
	multimap<int, Initialization*> initializations[InitializeStage::Num];

	InitializationManager() = default;
	void addInitialization(Initialization& initialization);
};