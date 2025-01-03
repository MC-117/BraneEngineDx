#pragma once
#ifndef _RENDER_H_
#define _RENDER_H_

#include "IRendering.h"
#include "Shape.h"

class Camera;
class SceneRenderData;
class CameraRenderData;
class ScreenHitData;
class MaterialRenderData;
class RenderGraph;

struct RenderInfo
{
	Camera* camera = NULL;
};

struct PreRenderInfo
{
	SceneRenderData* sceneData = NULL;
	Camera* camera = NULL;
};

class ENGINE_API Render : public IRendering
{
public:
	void* renderPool = NULL;
	int renderOrder = 0;
	bool hidden = false;
	bool isValid = true;
	bool isStatic = false;
	bool canCastShadow = false;
	bool customTransformSubmit = false;
	Matrix4f transformMat;

	Render();
	virtual ~Render();

	virtual void setBaseColor(Color color);
	virtual Color getBaseColor();

	virtual void preRender(PreRenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void postRender();
	virtual Matrix4f getTransformMatrix() const;
	virtual vector<Matrix4f>& getTransformMatrixs();
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape() const;
	virtual Material* getMaterial(unsigned int index = 0);
	virtual bool getMaterialEnable(unsigned int index = 0);
	virtual Shader* getShader() const;
	virtual RenderTarget* getShadowRenderTarget() const;
	virtual bool getCanCastShadow() const;
	virtual void* getRender() const;
};

#endif // !_RENDER_H_
