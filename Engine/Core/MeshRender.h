#pragma once
#ifndef _MESHRENDER_H_
#define _MESHRENDER_H_

#include "Mesh.h"
#include "Render.h"

struct MeshTransformData;

class ENGINE_API MeshRender : public Render
{
public:
	Mesh* mesh;
	bool frustumCulling = true;
	bool hasPrePass = false;
	vector<Material*> materials;
	vector<Material*> outlineMaterials;
	vector<bool> meshPartsEnable;
	vector<bool> outlineEnable;

	MeshRender();
	MeshRender(Mesh& mesh, Material& material);
	virtual ~MeshRender();

	virtual void setMesh(Mesh* mesh);
	virtual void setBaseColor(Color color);
	virtual Color getBaseColor();
	virtual Material* getMaterial(const string& name);
	virtual pair<string, Material*> getMaterial(int index);
	virtual bool getPartEnable(const string& name);
	virtual bool getPartEnable(int index);
	virtual bool setMaterial(const string& name, Material& material, bool all = false);
	virtual bool setMaterial(int index, Material& material);
	virtual bool setPartEnable(const string& name, bool enable, bool all = false);
	virtual bool setPartEnable(int index, bool enable);

	virtual void getMeshTransformData(MeshTransformData* data) const;

	virtual void fillMaterialsByDefault();

	virtual void preRender();
	virtual void render(RenderInfo& info);
	virtual Matrix4f getTransformMatrix() const;
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape() const;
	virtual Material* getMaterial(unsigned int index = 0);
	virtual bool getMaterialEnable(unsigned int index = 0);
	virtual Shader* getShader() const;
protected:
	virtual void remapMaterial();
};

#endif // !_MESHRENDER_H_