#pragma once

#include "Config.h"

enum class InitializeStage: short
{
	BeforeEngineConfig,
	BeforeEngineSetup,
	BeforePhysicsSetup,
	BeforeWindowSetup,
	BeforeRenderVendorSetup,
	BeforeAssetLoading,
	BeforeGameSetup,
	AfterEngineSetup,
	Num
};

enum class FinalizeStage : short
{
	BeforeEngineRelease,
	BeforeGameRelease,
	BeforePhysicsRelease,
	BeforeRenderVenderRelease,
	AfterRenderVenderRelease,
	AfterEngineRelease,
	Num
};

class InitializationManager;
class ENGINE_API Initialization
{
	friend InitializationManager;
protected:
	InitializeStage initializeStage = InitializeStage::BeforeAssetLoading;
	FinalizeStage finalizeStage = FinalizeStage::BeforeEngineRelease;
	int initializePriority = 0;
	int finalizePriority = 0;
	Initialization(InitializeStage initializeStage, int initializePriority, FinalizeStage finalizeStage, int finalizePriority);
	virtual ~Initialization() = default;
	virtual bool initialize() = 0;
	virtual bool finalize() = 0;
};

class ENGINE_API InitializationManager
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