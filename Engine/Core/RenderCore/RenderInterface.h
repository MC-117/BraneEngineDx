#pragma once
#include "../IRenderExecution.h"
#include "../GraphicType.h"
#include "../Serialization.h"
#include "ShaderResourceFixBindingPoint.h"

class IRenderContext;
class RenderTarget;
class Material;
class Texture;
struct MeshPart;
struct MeshData;
struct ShaderProgram;
class CameraRender;
class RenderTask;
class SceneRenderData;
class RenderCommandList;
class IGPUBuffer;

struct IRenderData
{
	long long usedFrame = -1;
	virtual void create() = 0;
	virtual void release() = 0;
	virtual void upload() = 0;
	virtual void bind(IRenderContext& context) = 0;
};

struct ISurfaceBuffer
{
	vector<bool> getterFlags;
	long long usedFrame = -1;

	virtual void create(CameraRender* cameraRender) = 0;
	virtual void resize(unsigned int width, unsigned int height) = 0;
	virtual void bind(IRenderContext& context) = 0;

	virtual RenderTarget* getRenderTarget() = 0;
	virtual Texture* getDepthTexture() = 0;
};

struct IRenderPack;

struct IBatchDrawCommandArray
{
	virtual ~IBatchDrawCommandArray() = default;
	virtual void bindInstanceBuffer(IRenderContext& context) = 0;
	virtual IGPUBuffer* getInstanceBuffer() = 0;
	virtual IGPUBuffer* getCommandBuffer() = 0;
	virtual unsigned int getInstanceCount() const = 0;
	virtual unsigned int getCommandCount() const = 0;
};

struct BatchDrawData
{
	IRenderData* transformData = NULL;
	IBatchDrawCommandArray* batchDrawCommandArray = NULL;

	bool isValid() const { return transformData && batchDrawCommandArray; }
};

template<class TransformRenderData, class CommandArray>
struct TBatchDrawData
{
	TransformRenderData* transformData = NULL;
	CommandArray* batchDrawCommandArray = NULL;

	operator BatchDrawData() const
	{
		return BatchDrawData{ transformData, batchDrawCommandArray };
	}

	bool isValid() const { return transformData && batchDrawCommandArray; }
};

struct IRenderCommand
{
	SceneRenderData* sceneData = NULL;
	BatchDrawData batchDrawData;
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
	BatchDrawData batchDrawData;
	RenderTarget* renderTarget;
	IRenderData* cameraData;
	ShaderProgram* shaderProgram;
	RenderMode renderMode;
	IRenderData* materialData;
	MeshData* meshData;
};

struct IRenderPack
{
	virtual ~IRenderPack();
	virtual bool setRenderCommand(const IRenderCommand& command) = 0;
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext) = 0;
	virtual void reset() = 0;
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

class IRenderDataCollector
{
public:
	virtual void add(IRenderData& data) = 0;
	virtual void clear() = 0;
	virtual void upload() = 0;
};

class BaseRenderDataCollector : public IRenderDataCollector
{
public:
	virtual void add(IRenderData& data);
	virtual void clear();
	virtual void upload();
protected:
	unordered_set<IRenderData*> collection;
};

class ENGINE_API RenderGraph : public Serializable
{
public:
	Serialize(RenderGraph,);

	unordered_set<SceneRenderData*> sceneDatas;

	virtual ISurfaceBuffer* newSurfaceBuffer() = 0;
	virtual bool setRenderCommand(const IRenderCommand& cmd) = 0;
	virtual void setImGuiDrawData(ImDrawData* drawData) = 0;
	virtual void addPass(RenderPass& pass) = 0;
	virtual void prepare() = 0;
	virtual void execute(IRenderContext& context) = 0;
	virtual void reset() = 0;

	virtual IRenderDataCollector* getRenderDataCollector() = 0;
	virtual void getPasses(vector<pair<string, RenderPass*>>& passes);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from) { return true; }
	virtual bool serialize(SerializationInfo& to) { serializeInit(this, to); return true; }
};