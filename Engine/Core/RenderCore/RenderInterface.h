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
struct Shader;
struct ShaderProgram;
class CameraRender;
class RenderTask;
class CameraRenderData;
class SceneRenderData;
class MaterialRenderData;
class RenderCommandList;
class IGPUBuffer;
class IMaterial;

struct IUpdateableRenderData
{
	long long usedFrameMainThread = -1;
	long long usedFrameRenderThread = -1;
	virtual void updateMainThread() = 0;
	virtual void updateRenderThread() = 0;
};

struct IRenderData : IUpdateableRenderData
{
	virtual void updateMainThread();
	virtual void updateRenderThread();

	virtual void create() = 0;
	virtual void release() = 0;
	virtual void upload() = 0;
	virtual void bind(IRenderContext& context) = 0;
};

struct ISurfaceBuffer
{
	vector<bool> getterFlags;
	long long usedFrame = -1;

	virtual void create(CameraRenderData* cameraRenderData) = 0;
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

	virtual void clean() {}
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

	template<class OtherTransformRenderData, class OtherCommandArray>
	TBatchDrawData& operator=(const TBatchDrawData<OtherTransformRenderData, OtherCommandArray>& other)
	{
		transformData = other.transformData;
		batchDrawCommandArray = other.batchDrawCommandArray;
		return *this;
	}
};

struct MaterialDrawData
{
	Shader* shader = NULL;
	IRenderData* renderData = NULL;
	int renderOrder = 0;
	bool canCastShadow = true;
	size_t drawKey = 0;

	MaterialDrawData() = default;
	MaterialDrawData(Material* material);

	bool isValid() const;
};

class IRenderDataCollector;

struct IRenderCommand
{
	SceneRenderData* sceneData = NULL;
	BatchDrawData batchDrawData;
	MaterialRenderData* materialRenderData = NULL;
	MeshPart* mesh = NULL;
	list<IRenderData*> bindings;
	virtual bool isValid() const = 0;
	virtual Enum<ShaderFeature> getShaderFeature() const = 0;
	virtual RenderMode getRenderMode() const = 0;
	virtual bool canCastShadow() const = 0;
	virtual void collectRenderData(IRenderDataCollector* collectorMainThread, IRenderDataCollector* collectorRenderThread);
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
	IMaterial* materialVariant;
	MeshData* meshData;
	uint8_t stencilValue;
};

struct IRenderPack
{
	virtual ~IRenderPack();
	virtual bool setRenderCommand(const IRenderCommand& command) = 0;
	virtual void excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext) = 0;
	virtual void reset() = 0;
};

class RenderGraph;

class RenderPass
{
public:
	RenderGraph* renderGraph = NULL;

	virtual bool loadDefaultResource() = 0;

	virtual void prepare() = 0;
	virtual void execute(IRenderContext& context) = 0;
	virtual void reset() = 0;

	virtual void getOutputTextures(vector<pair<string, Texture*>>& textures);
};

class IRenderDataCollector
{
public:
	virtual void add(IUpdateableRenderData& data) = 0;
	virtual void clear() = 0;
	virtual void updateMainThread(long long mainFrame) = 0;
	virtual void updateRenderThread(long long renderFrame) = 0;
};

class BaseRenderDataCollector : public IRenderDataCollector
{
public:
	virtual void add(IUpdateableRenderData& data);
	virtual void clear();
	virtual void updateMainThread(long long mainFrame);
	virtual void updateRenderThread(long long renderFrame);
protected:
	unordered_set<IUpdateableRenderData*> collection;
};

class ENGINE_API RenderGraph : public Serializable
{
public:
	Serialize(RenderGraph,);

	unordered_set<SceneRenderData*> sceneDatas;

	virtual bool loadDefaultResource() = 0;

	virtual ISurfaceBuffer* newSurfaceBuffer() = 0;
	virtual bool setRenderCommand(const IRenderCommand& cmd) = 0;
	virtual void setImGuiDrawData(ImDrawData* drawData) = 0;
	virtual void addPass(RenderPass& pass) = 0;
	virtual void prepare() = 0;
	virtual void execute(IRenderContext& context, long long renderFrame) = 0;
	virtual void reset() = 0;

	virtual IRenderDataCollector* getRenderDataCollectorMainThread() = 0;
	virtual IRenderDataCollector* getRenderDataCollectorRenderThread() = 0;
	virtual void getPasses(vector<pair<string, RenderPass*>>& passes);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from) { return true; }
	virtual bool serialize(SerializationInfo& to) { serializeInit(this, to); return true; }
};