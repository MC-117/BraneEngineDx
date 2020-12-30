#pragma once
#ifndef _MESHRENDER_H_
#define _MESHRENDER_H_

#include "Mesh.h"
#include "Render.h"

class MeshRender : public Render
{
public:
	Mesh& mesh;
	bool enableOutline = false;
	Material* outlineMaterial = NULL;
	vector<Material*> materials;

	MeshRender(Mesh& mesh, Material& material);
	virtual ~MeshRender();

	virtual void setBaseColor(Color color);
	virtual Color getBaseColor();
	virtual Material* getMaterial(const string& name);
	virtual pair<string, Material*> getMaterial(int index);
	virtual bool setMaterial(const string& name, Material& material, bool all = false);
	virtual bool setMaterial(int index, Material& material);
	virtual void fillMaterialsByDefault();

	virtual void preRender();
	virtual void render(RenderInfo& info);
	virtual Matrix4f getTransformMatrix() const;
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape() const;
	virtual Material* getMaterial(unsigned int index = 0);
	virtual Shader* getShader() const;
	virtual void setupRenderResource();
protected:
	virtual void remapMaterial();
};

#endif // !_MESHRENDER_H_