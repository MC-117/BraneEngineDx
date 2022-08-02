#include "Spine2DConfig.h"

static spine::DebugExtension* debugExtension = NULL;

spine::SpineExtension* spine::getDefaultExtension()
{
	debugExtension = new DebugExtension(new DefaultSpineExtension());
	return debugExtension;
}

Spine2DInitialization Spine2DInitialization::instance;

Spine2DInitialization::Spine2DInitialization() : Initialization(0)
{
}

bool Spine2DInitialization::initialze()
{
    return true;
}
