#pragma once
#include "../IRenderExecution.h"
#include "../GraphicType.h"
#include "../Serialization.h"

class IRenderContext;
class RenderTarget;
class Material;
class Texture;
struct MeshPart;
struct MeshData;
struct ShaderProgram;
class RenderTask;
class SceneRenderData;
class RenderCommandList;

struct IRenderData
{
	long long usedFrame = -1;
	virtual void create() = 0;
	virtual void release() = 0;
	virtual void upload() = 0;
	virtual void bind(IRenderContext& context) = 0;
};

struct IRenderPack;

struct IRenderCommand
{
	SceneRenderData* sceneData = NULL;
	Material* material = NULL;
	MeshPart* mesh = NULL;
	list<IRenderData*> bindings;
	virtual bool isValid() const = 0;
	virtual Enum<ShaderFeature> getShaderFeature() const = 0;
	virtual RenderMode getRenderMode() const = 0;
	virtual bool canCastShadow() const = 0;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const = 0;
};

struct RenderTaskContext
{
	SceneRenderData* sceneData;
	IRenderData* cameraData;
	ShaderProgram* shaderProgram;
	RenderMode renderMode;
	IRenderData* materialData;
	MeshData* meshData;
};

struct IRenderPack
{
	IRenderExecution* vendorRenderExecution = NULL;
	virtual ~IRenderPack();
	virtual bool setRenderCommand(const IRenderCommand& command) = 0;
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext) = 0;
	virtual void newVendorRenderExecution();
};

class RenderGraph;

class RenderPass
{
public:
	RenderGraph* renderGraph = NULL;

	virtual void prepare() = 0;
	virtual void execute(IRenderContext& context) = 0;
	virtual void reset() = 0;

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
};

class RenderGraph : public Serializable
{
public:
	Serialize(RenderGraph,);

	unordered_set<SceneRenderData*> sceneDatas;

	virtual bool setRenderCommand(const IRenderCommand& cmd) = 0;
	virtual void setImGuiDrawData(ImDrawData* drawData) = 0;
	virtual void addPass(RenderPass& pass) = 0;
	virtual void prepare() = 0;
	virtual void execute(IRenderContext& context) = 0;
	virtual void reset() = 0;

	virtual void getPasses(vector<pair<string, RenderPass*>>& passes);

	static Serializable* instantiate(const SerializationInfo& from);
};