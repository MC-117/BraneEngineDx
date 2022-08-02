#pragma once

#include "../Core/Config.h"
#include "../Core/Console.h"
#include "../Core/MeshData.h"
#include "../Core/Mesh.h"
#include "../Core/GPUBuffer.h"

#undef min
#undef max
#include <spine/spine.h>
#include <spine/Extension.h>
#include <spine/Debug.h>

#include "../Core/InitializationManager.h"

spine::SpineExtension* spine::getDefaultExtension();

class Spine2DInitialization : public Initialization
{
protected:
	static Spine2DInitialization instance;
	Spine2DInitialization();
	virtual bool initialze();
};