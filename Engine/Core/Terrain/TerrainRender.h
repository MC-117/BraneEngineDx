#pragma once

#include "../Render.h"
#include "TerrainGeometry.h"

struct TerrainRenderData;

class ENGINE_API TerrainRender : public Render
{
public:
	TerrainGeometry* geometry = NULL;
	Material* material = NULL;

	TerrainRender();

	void setGeometry(TerrainGeometry* geometry);
	TerrainGeometry* getGeometry() const;

	virtual void setInstanceInfo(unsigned int instanceID, unsigned int instanceCount);

	virtual void getMeshTransformData(MeshTransformData* data) const;

	virtual void preRender(PreRenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual Matrix4f getTransformMatrix() const;
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape() const;
	virtual Material* getMaterial(unsigned int index = 0);
	virtual bool getMaterialEnable(unsigned int index = 0);
	virtual unsigned int getInstanceID() const;
	virtual unsigned int getInstanceCount() const;
	virtual Shader* getShader() const;

	TerrainRenderData* getRenderData();
protected:
	unsigned int instanceID = -1;
	unsigned int instanceCount = -1;

	TerrainRenderData* terrainRenderData = NULL;
};
