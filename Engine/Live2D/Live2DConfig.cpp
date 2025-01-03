#include "Live2DConfig.h"
#include "Live2DConsole.h"
#include "Live2DAllocator.h"

Live2DInitialization Live2DInitialization::instance;

Csm::CubismFramework::Option option;
Live2DAllocator allocator;

Live2DInitialization::Live2DInitialization() : Initialization(
    InitializeStage::BeforeAssetLoading, 0,
    FinalizeStage::BeforeRenderVenderRelease, 0)
{
    option.LogFunction = CubismPrintMessage_Impl;
    option.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;
}

bool Live2DInitialization::initialize()
{
    Csm::CubismFramework::StartUp(&::allocator, &option);
    //Initialize cubism
    Csm::CubismFramework::Initialize();
    return true;
}

bool Live2DInitialization::finalize()
{
    //Dispose cubism
    Csm::CubismFramework::Dispose();
    Csm::CubismFramework::CleanUp();
    return true;
}
