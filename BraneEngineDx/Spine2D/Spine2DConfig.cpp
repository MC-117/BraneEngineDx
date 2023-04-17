#include "Spine2DConfig.h"

static spine::DebugExtension* debugExtension = NULL;

spine::SpineExtension* spine::getDefaultExtension()
{
	debugExtension = new DebugExtension(new DefaultSpineExtension());
	return debugExtension;
}

Spine2DInitialization Spine2DInitialization::instance;

Spine2DInitialization::Spine2DInitialization() : Initialization(
	InitializeStage::BeforeAssetLoading, 0,
	FinalizeStage::BeforeRenderVenderRelease, 0)
{
}

bool Spine2DInitialization::initialize()
{
    return true;
}

bool Spine2DInitialization::finalize()
{
	return true;
}
