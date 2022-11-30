#pragma once

#include "../Render.h"
#include "TerrainGeometry.h"

class TerrainRender : public Render
{
public:
	TerrainGeometry* geometry = NULL;
	Material* material = NULL;

	TerrainRender();

	void setGeometry(TerrainGeometry* geometry);
	TerrainGeometry* getGeometry() const;

	virtual void preRender();
	virtual void render(RenderInfo& info);
	virtual Matrix4f getTransformMatrix() const;
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape() const;
	virtual Material* getMaterial(unsigned int index = 0);
	virtual bool getMaterialEnable(unsigned int index = 0);
	virtual Shader* getShader() const;
};