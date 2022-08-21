#pragma once
#include "../IRenderExecution.h"
#include "../Serialization.h"

class IRenderContext;
class RenderTarget;
class RenderCommandList;

struct IRenderData
{
	long long usedFrame = -1;
	virtual void create() = 0;
	virtual void release() = 0;
	virtual void upload() = 0;
	virtual void bind(IRenderContext& context) = 0;
};

struct IRenderPack
{
	IRenderExecution* vendorRenderExecution = NULL;
	virtual ~IRenderPack();
	virtual void excute(IRenderContext& context) = 0;
	virtual void newVendorRenderExecution();
};

class RenderPass
{
public:
	virtual void prepare() = 0;
	virtual void execute(IRenderContext& context) = 0;
};

class RenderGraph : public Serializable
{
public:
	Serialize(RenderGraph,);

	virtual void setRenderCommandList(RenderCommandList& commandList) = 0;
	virtual void setMainRenderTarget(RenderTarget& renderTarget) = 0;
	virtual void setImGuiDrawData(ImDrawData* drawData) = 0;
	virtual void addPass(RenderPass& pass) = 0;
	virtual void prepare() = 0;
	virtual void execute(IRenderContext& context) = 0;
	virtual void reset() = 0;

	static Serializable* instantiate(const SerializationInfo& from);
};