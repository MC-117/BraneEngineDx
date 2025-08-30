#pragma once
#ifndef _POSTPROCESSPASS_H_
#define _POSTPROCESSPASS_H_

#include "../Render.h"

struct EditorInfo;

struct PostProcessResource
{
	CameraRenderData* cameraRenderData;

	Texture* screenTexture;
	RenderTarget* screenRenderTarget;

	Texture* depthTexture;
	RenderTarget* depthRenderTarget;

	Texture* bloomTexture;
	RenderTarget* bloomRenderTarget;

	Texture* ssaoTexture;
	RenderTarget* ssaoRenderTarget;

	Texture* volumetricFogTexture;
	RenderTarget* volumetricFogRenderTarget;

	Texture* blurTexture;

	RenderTarget* finalRenderTarget;

	unordered_map<string, Texture*> textures;
	unordered_map<string, RenderTarget*> renderTargets;

	PostProcessResource();
	
	bool setTexture(const string& passName, const string& name, Texture& texture, bool overwrite = false);
	Texture* getTexture(const string& name);

	void reset();
};

class PostProcessPass : public RenderPass, public Serializable
{
public:
	Serialize(PostProcessPass,);

	PostProcessPass(const Name& name = Name::none, Material* material = NULL);
	virtual ~PostProcessPass();

	void setResource(PostProcessResource& resource);
	void setMaterial(Material* material);
	Material* getMaterial();

	void setEnable(bool enable);
	bool getEnable();

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);

	virtual void onGUI(EditorInfo& info);
	virtual void reset();

	virtual Name getPassName() const;

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Unit2Di size = { 1280, 720 };
	PostProcessResource* resource = NULL;
	Name name;
	Material* material = NULL;
	MaterialRenderData* materialRenderData = NULL;
	IMaterial* materialVaraint = NULL;
	IPipelineState* pipelineState = NULL;
	IRenderData* cameraRenderData = NULL;
	bool enable = false;
};

#endif // !_POSTPROCESSPASS_H_

