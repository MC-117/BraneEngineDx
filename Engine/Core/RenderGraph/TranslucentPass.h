#pragma once

#include "../RenderCore/RenderCommandList.h"
#include "SSRBindings.h"

struct TranslucentSSRBinding : public SSRBinding
{
	Texture* gBufferA;
	Texture* hiZTexture;

	TranslucentSSRBinding();

	virtual void create();
	virtual void release();
	virtual void bind(IRenderContext& context);
};

class TranslucentPass : public RenderPass
{
public:
	RenderCommandList commandList;
	Enum<ClearFlags> plusClearFlags = Clear_None;
	Enum<ClearFlags> minusClearFlags = Clear_None;

	virtual bool setRenderCommand(const IRenderCommand& command);

	virtual bool loadDefaultResource();
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
protected:
	Timer timer;
	vector<shared_ptr<TranslucentSSRBinding>> cachedSSRBindings;
	unordered_map<size_t, shared_ptr<TranslucentSSRBinding>> ssrBindings;
	vector<pair<string, Texture*>> outputTextures;

	TranslucentSSRBinding* getSSRBinding(const SceneRenderData& sceneData, const CameraRenderData& cameraData);
};