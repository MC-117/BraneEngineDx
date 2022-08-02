#pragma once
#ifndef _RENDER_H_
#define _RENDER_H_

#include "IRendering.h"

class Camera;

struct RenderInfo
{
	enum RenderMode
	{
		Forward, Deferred
	};
	
	Matrix4f projectionViewMat;
	Matrix4f lightSpaceMat;
	Vector3f cameraLoc;
	Vector3f cameraDir;
	Unit2Di viewSize;
	float fovy;
	RenderMode mode;
	bool depthWrite = true;
	bool alphaTest = false;
	RenderCommandList* cmdList = NULL;
	Camera* camera = NULL;
	IRendering* tempRender = NULL;
	list<IRendering*> taskList;

	RenderInfo(Matrix4f projectionViewMat = Matrix4f::Identity(),
		Matrix4f lightSpaceMat = Matrix4f::Identity(),
		Vector3f cameraLoc = Vector3f::Identity(),
		Vector3f cameraDir = Vector3f::Identity(),
		Unit2Di viewSize = { 0, 0 },
		float fovy = 0,
		RenderMode mode = Forward,
		bool depthWrite = true,
		bool alphaTest = false);

	RenderInfo(const RenderInfo& info);

	RenderInfo& operator=(const RenderInfo& info);
};

class Render : public IRendering
{
public:
	void* renderPool = NULL;
	int renderOrder = 0;
	bool hidden = false;
	bool isValid = true;
	bool isStatic = false;
	bool canCastShadow = false;
	bool customTransformSubmit = false;
	bool customRenaderSubmit = false;
	unsigned int instanceID = -1;
	unsigned int instanceCount = 1;
	Matrix4f transformMat;

	Render();
	virtual ~Render();

	virtual void setBaseColor(Color color);
	virtual Color getBaseColor();

	virtual void preRender();
	virtual void render(RenderInfo& info);
	virtual void postRender();
	virtual Matrix4f getTransformMatrix() const;
	virtual vector<Matrix4f>& getTransformMatrixs();
	virtual Matrix4f getLightSpaceMatrix() const;
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape() const;
	virtual Material* getMaterial(unsigned int index = 0);
	virtual bool getMaterialEnable(unsigned int index = 0);
	virtual Shader* getShader() const;
	virtual RenderTarget* getShadowRenderTarget() const;
	virtual bool getCanCastShadow() const;
	virtual unsigned int getInstanceID() const;
	virtual unsigned int getInstanceCount() const;
	virtual void* getRender() const;
};

#endif // !_RENDER_H_
