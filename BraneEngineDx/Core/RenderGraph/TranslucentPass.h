#pragma once

#include "../RenderCore/RenderCommandList.h"

struct TranslucentSSRBinding : public IRenderData
{
	struct SSRInfo
	{
		Vector4f hiZUVScale;
		int hiZStartLevel;
		int hiZStopLevel;
		int hiZMaxStep;
		int pad;
	} ssrInfo;

	Texture* gBufferA;
	Texture* gBufferB;
	Texture* hiZTexture;
	GPUBuffer ssrInfoBuffer;

	TranslucentSSRBinding();

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};

class TranslucentPass : public RenderPass
{
public:
	RenderCommandList commandList;
	Enum<ClearFlags> plusClearFlags = Clear_None;
	Enum<ClearFlags> minusClearFlags = Clear_None;

	virtual bool setRenderCommand(const IRenderCommand& command);
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