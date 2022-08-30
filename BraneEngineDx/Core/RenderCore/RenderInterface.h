#pragma once
#include "../IRenderExecution.h"
#include "../GraphicType.h"
#include "../Serialization.h"

class IRenderContext;
class RenderTarget;
class Camera;
class Material;
struct MeshPart;
struct MeshData;
struct ShaderProgram;
class RenderTask;
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
	Camera* camera;
	Material* material;
	MeshPart* mesh;
	list<IRenderData*> bindings;
	virtual bool isValid() const = 0;
	virtual Enum<ShaderFeature> getShaderFeature() const = 0;
	virtual RenderMode getRenderMode() const = 0;
	virtual IRenderPack* createRenderPack(RenderCommandList& commandList) const = 0;
};

struct RenderTaskContext
{
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