#include "InitializationManager.h"

Initialization::Initialization(InitializeStage stage, int priority) : stage(stage), priority(priority)
{
    InitializationManager::instance().addInitialization(*this);
}

bool Initialization::initialze()
{
    return false;
}

InitializationManager& InitializationManager::instance()
{
    static InitializationManager self;
    return self;
}

void InitializationManager::initialze(InitializeStage stage)
{
    auto& range = initializations[stage];
    for (auto b = range.begin(), e = range.end(); b != e; b++)
        b->second->initialze();
}

void InitializationManager::addInitialization(Initialization& initialization)
{
    initializations[initialization.stage].insert(make_pair(initialization.priority, &initialization));
}
