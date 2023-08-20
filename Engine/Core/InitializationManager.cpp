#include "InitializationManager.h"

Initialization::Initialization(
    InitializeStage initializeStage,
    int initializePriority,
    FinalizeStage finalizeStage,
    int finalizePriority) 
    : initializeStage(initializeStage)
    , initializePriority(initializePriority)
    , finalizeStage(finalizeStage)
    , finalizePriority(finalizePriority)
{
    InitializationManager::instance().addInitialization(*this);
}

InitializationManager& InitializationManager::instance()
{
    static InitializationManager self;
    return self;
}

void InitializationManager::initialize(InitializeStage stage)
{
    auto& range = initializations[(int)stage];
    for (auto b = range.begin(), e = range.end(); b != e; b++)
        b->second->initialize();
}

void InitializationManager::finalize(FinalizeStage stage)
{
    auto& range = finalizations[(int)stage];
    for (auto b = range.begin(), e = range.end(); b != e; b++)
        b->second->finalize();
}

void InitializationManager::addInitialization(Initialization& initialization)
{
    initializations[(int)initialization.initializeStage].insert(make_pair(initialization.initializePriority, &initialization));
    finalizations[(int)initialization.finalizeStage].insert(make_pair(initialization.finalizePriority, &initialization));
}
