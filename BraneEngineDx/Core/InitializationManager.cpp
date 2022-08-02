#include "InitializationManager.h"

Initialization::Initialization(int priority) : priority(priority)
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

void InitializationManager::initialze()
{
    for (auto b = initializations.begin(), e = initializations.end(); b != e; b++)
        b->second->initialze();
}

void InitializationManager::addInitialization(Initialization& initialization)
{
    initializations.insert(make_pair(initialization.priority, &initialization));
}
