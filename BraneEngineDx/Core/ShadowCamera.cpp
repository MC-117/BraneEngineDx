#include "ShadowCamera.h"
#include "Asset.h"

ShadowCamera::ShadowCamera(Unit2Di size, string name) : Camera(shadowCameraRender, name), shadowCameraRender(Material::defaultDepthMaterial)
{
	//setSize(size);
}