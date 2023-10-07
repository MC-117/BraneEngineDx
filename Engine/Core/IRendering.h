#pragma once
#ifndef _IRENDERING_H_
#define _IRENDERING_H_

#include "Unit.h"
#include "Shader.h"
#include "RenderTarget.h"
#include "Material.h"
#include "RenderCore/CameraData.h"

class ENGINE_API IRendering
{
public:
	enum RenderType
	{
		None_Render, Normal_Render, Camera_Render, Light_Render, Particle_Render, SceneCapture_Render
	};

	virtual ~IRendering() {}
	virtual Matrix4f getTransformMatrix() const = 0;
	virtual vector<Matrix4f>& getTransformMatrixs() = 0;
	virtual RenderType getRenderType() const = 0;
	virtual Shader* getShader() const = 0;
	virtual RenderTarget* getShadowRenderTarget() const = 0;
	virtual bool getCanCastShadow() const = 0;
	virtual unsigned int getInstanceID() const = 0;
	virtual unsigned int getInstanceCount() const = 0;
	virtual void* getRender() const = 0;
};

#endif // !_IRENDERING_H_