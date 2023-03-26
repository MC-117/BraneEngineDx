#include "ProfileCore.h"

IProfiler::IProfiler()
{
    ProfilerManager::instance().registProfilor(*this);
}

ProfilerManager& ProfilerManager::instance()
{
    static ProfilerManager manager;
    return manager;
}

void ProfilerManager::tick()
{
    for (auto profilor : profilors)
        profilor->tick();
}

ProfilerManager::ProfilerManager() : Initialization(InitializeStage::BeforeEngineSetup, 0)
{
}

ProfilerManager::~ProfilerManager()
{
    for (auto profilor : profilors)
        profilor->release();
    profilors.clear();
}

bool ProfilerManager::initialze()
{
    for (auto profilor : profilors)
        profilor->init();
    return true;
}

void ProfilerManager::registProfilor(IProfiler& profilor)
{
    profilors.insert(&profilor);
}

void ProfilerManager::setCapture()
{
    for (auto profilor : profilors)
        profilor->setCapture();
}

void ProfilerManager::setNextCapture()
{
    for (auto profilor : profilors)
        profilor->setNextCapture();
}

void ProfilerManager::beginScope(const string& name)
{
    for (auto profilor : profilors)
        profilor->beginScope(name);
}

void ProfilerManager::endScope()
{
    for (auto profilor : profilors)
        profilor->endScope();
}

void ProfilerManager::beginFrame()
{
    for (auto profilor : profilors)
        profilor->beginFrame();
}

void ProfilerManager::endFrame()
{
    for (auto profilor : profilors)
        profilor->endFrame();
}
