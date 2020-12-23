#pragma once
#ifndef _IRENDERING_H_
#define _IRENDERING_H_

#include "Unit.h"
#include "RenderCommandList.h"

class IRendering
{
public:
	enum RenderType
	{
		None, Normal, Light, Camera, PostProcess, Particles
	};

	virtual ~IRendering() {}
	virtual Matrix4f getTransformMatrix() const = 0;
	virtual vector<Matrix4f>& getTransformMatrixs() { return vector<Matrix4f>(); }
	virtual Matrix4f getLightSpaceMatrix() const = 0;
	virtual RenderType getRenderType() const = 0;
	virtual Shape* getShape() const = 0;
	virtual Material* getMaterial(unsigned int index = 0) = 0;
	virtual Shader* getShader() const = 0;
	virtual RenderTarget* getShadowRenderTarget() const = 0;
	virtual bool getCanCastShadow() const = 0;
	virtual unsigned int getInstanceID() const = 0;
	virtual unsigned int getInstanceCount() const = 0;
	virtual void* getRender() const = 0;
	virtual void setupRenderResource() = 0;
	virtual void setSourceRenderTarget(RenderTarget& renderTarget) = 0;
};

#endif // !_IRENDERING_H_