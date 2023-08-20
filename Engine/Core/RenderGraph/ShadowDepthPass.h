#pragma once

#include "MeshPass.h"

class ShadowDepthPass : public MeshPass
{
public:
	virtual bool setRenderCommand(const IRenderCommand& cmd);
protected:
	Material* defaultDepthMaterial;
};