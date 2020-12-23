#pragma once
#ifndef _CAMERARENDER_H_
#define _CAMERARENDER_H_

#include "Render.h"

class CameraRender : public Render
{
public:
	Unit2Di size = { 0, 0 };
	Material& material;
	RenderTarget& renderTarget;

	Matrix4f projectionViewMat = Matrix4f::Identity();
	Vector3f cameraLoc = Vector3f::Identity();
	Vector3f cameraDir = Vector3f::Identity();
	Vector3f cameraUp = Vector3f::Identity();
	Vector3f cameraLeft = Vector3f::Identity();

	CameraRender(RenderTarget& renderTarget = RenderTarget::defaultRenderTarget, Material& material = Material::nullMaterial);
	virtual ~CameraRender();
	
	virtual Texture2D* getSceneBlurTex();

	virtual void setSize(Unit2Di size);
	virtual void addExtraRenderTex(string& name, const Texture2DInfo& info = Texture2DInfo());
	virtual void addExtraRenderTex(const Texture2DInfo& info = Texture2DInfo(), unsigned int num = 1);

	virtual bool isValid();
	virtual IRendering::RenderType getRenderType() const;
	virtual void preRender();
	virtual void render(RenderInfo& info);
protected:
	list<Texture2D*> extraRenderTex;
};

#endif // !_CAMERARENDER_H_
