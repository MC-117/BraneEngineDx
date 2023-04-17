#pragma once

#include "Config.h"

enum class InitializeStage: short
{
	BeforeEngineConfig,
	BeforeEngineSetup,
	BeforeScriptSetup,
	BeforePhysicsSetup,
	BeforeWindowSetup,
	BeforeRenderVendorSetup,
	BeforeAssetLoading,
	AfterEngineSetup,
	Num
};

enum class FinalizeStage : short
{
	BeforeEngineRelease,
	BeforeScriptRelease,
	BeforePhysicsRelease,
	BeforeRenderVenderRelease,
	AfterRenderVenderRelease,
	AfterEngineRelease,
	Num
};

class InitializationManager;
class Initialization
{
	friend InitializationManager;
protected:
	InitializeStage initializeStage = InitializeStage::BeforeAssetLoading;
	FinalizeStage finalizeStage = FinalizeStage::BeforeEngineRelease;
	int initializePriority = 0;
	int finalizePriority = 0;
	Initialization(InitializeStage initializeStage, int initializePriority, FinalizeStage finalizeStage, int finalizePriority);
	virtual bool initialize() = 0;
	virtual bool finalize() = 0;
};

class InitializationManager
{
	friend Initialization;
public:
	static InitializationManager& instance();
	void initialize(InitializeStage stage);
	void finalize(FinalizeStage stage);
protected:
	multimap<int, Initialization*> initializations[(int)InitializeStage::Num];
	multimap<int, Initialization*> finalizations[(int)FinalizeStage::Num];

	InitializationManager() = default;
	void addInitialization(Initialization& initialization);
};