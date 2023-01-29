#pragma once
#ifndef _POSTPROCESSPASS_H_
#define _POSTPROCESSPASS_H_

#include "../Render.h"

struct EditorInfo;

struct PostProcessResource
{
	SceneRenderData* sceneRenderData = NULL;
	CameraRenderData* cameraRenderData = NULL;

	Texture* screenTexture = NULL;
	RenderTarget* screenRenderTarget = NULL;

	Texture* depthTexture = NULL;
	RenderTarget* depthRenderTarget = NULL;

	Texture* bloomTexture = NULL;
	RenderTarget* bloomRenderTarget = NULL;

	Texture* ssaoTexture = NULL;
	RenderTarget* ssaoRenderTarget = NULL;

	Texture* volumetricFogTexture = NULL;
	RenderTarget* volumetricFogRenderTarget = NULL;

	Texture* blurTexture = NULL;

	RenderTarget* finalRenderTarget = &RenderTarget::defaultRenderTarget;

	unordered_map<string, Texture*> textures;
	unordered_map<string, RenderTarget*> renderTargets;
	
	bool setTexture(const string& passName, const string& name, Texture& texture, bool overwrite = false);
	Texture* getTexture(const string& name);

	void reset();
};

class PostProcessPass : public RenderPass, public Serializable
{
public:
	Serialize(PostProcessPass,);

	PostProcessPass(const string& name = "None", Material* material = NULL);
	virtual ~PostProcessPass();

	void setResource(PostProcessResource& resource);
	void setMaterial(Material* material);
	Material* getMaterial();

	void setEnable(bool enable);
	bool getEnable();

	string getName();

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);

	virtual void onGUI(EditorInfo& info);
	virtual void reset();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Unit2Di size = { 1280, 720 };
	PostProcessResource* resource = NULL;
	string name;
	Material* material = NULL;
	ShaderProgram* program = NULL;
	IRenderData* materialRenderData = NULL;
	IRenderData* cameraRenderData = NULL;
	bool enable = false;
};

#endif // !_POSTPROCESSPASS_H_

