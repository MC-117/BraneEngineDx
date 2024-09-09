#include "Spine2DConfig.h"

class MiSpineExtension : public spine::DefaultSpineExtension
{
protected:
	virtual void *_alloc(size_t size, const char *file, int line)
	{
		return mi_malloc(size);
	}

	virtual void *_calloc(size_t size, const char *file, int line)
	{
		return mi_calloc(1, size);
	}

	virtual void *_realloc(void *ptr, size_t size, const char *file, int line)
	{
		return mi_realloc(ptr, size);
	}

	virtual void _free(void *mem, const char *file, int line)
	{
		return mi_free(mem);
	}
};

spine::SpineExtension* spine::getDefaultExtension()
{
	static MiSpineExtension miExtension;
	return &miExtension;
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
