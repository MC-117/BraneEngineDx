#include "ProfileCore.h"

IProfilor::IProfilor()
{
    ProfilorManager::instance().registProfilor(*this);
}

ProfilorManager& ProfilorManager::instance()
{
    static ProfilorManager manager;
    return manager;
}

void ProfilorManager::tick()
{
    for (auto profilor : profilors)
        profilor->tick();
}

ProfilorManager::ProfilorManager() : Initialization(InitializeStage::BeforeEngineSetup, 0)
{
}

ProfilorManager::~ProfilorManager()
{
    for (auto profilor : profilors)
        profilor->release();
    profilors.clear();
}

bool ProfilorManager::initialze()
{
    for (auto profilor : profilors)
        profilor->init();
    return true;
}

void ProfilorManager::registProfilor(IProfilor& profilor)
{
    profilors.insert(&profilor);
}

void ProfilorManager::setCapture()
{
    for (auto profilor : profilors)
        profilor->setCapture();
}

void ProfilorManager::beginScope(const string& name)
{
    for (auto profilor : profilors)
        profilor->beginScope(name);
}

void ProfilorManager::endScope()
{
    for (auto profilor : profilors)
        profilor->endScope();
}

void ProfilorManager::beginFrame()
{
    for (auto profilor : profilors)
        profilor->beginFrame();
}

void ProfilorManager::endFrame()
{
    for (auto profilor : profilors)
        profilor->endFrame();
}
