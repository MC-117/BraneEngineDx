#pragma once
#ifndef _IRENDERING_H_
#define _IRENDERING_H_

#include "Unit.h"
#include "RenderCore/RenderCommandList.h"

class RenderResource
{
public:
	int instanceID;
	int instanceIDCount;
	bool enable;
	Material* material;
	MeshPart* meshPart;

	virtual ~RenderResource() = default;
	virtual bool isValid() const;
};

class IRendering
{
public:
	enum RenderType
	{
		None, Normal, Light, Camera, PostProcess, Particles, PreProcess
	};

	virtual ~IRendering() {}
	virtual Matrix4f getTransformMatrix() const = 0;
	virtual vector<Matrix4f>& getTransformMatrixs() { return vector<Matrix4f>(); }
	virtual Matrix4f getLightSpaceMatrix() const = 0;
	virtual RenderType getRenderType() const = 0;
	virtual Shader* getShader() const = 0;
	virtual RenderTarget* getShadowRenderTarget() const = 0;
	virtual bool getCanCastShadow() const = 0;
	virtual unsigned int getInstanceID() const = 0;
	virtual unsigned int getInstanceCount() const = 0;
	virtual void* getRender() const = 0;
	virtual int getRenderResource(vector<RenderResource>& resources) { return 0; };
};

#endif // !_IRENDERING_H_