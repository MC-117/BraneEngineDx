#pragma once
#include "../GPUBuffer.h"
#include "../RenderCore/RenderInterface.h"
#include "../MeshActor.h"

struct ColorBufferTestRenderData : public IRenderData
{
	Color color;
	vector<Color> colors;
	Vector2u size = { 1024, 1024 };
	bool needUpdate = false;
	GPUBuffer colorBuffer = GPUBuffer(GB_Storage, GBF_Float4);

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};

class ColorBufferTestMeshRender : public MeshRender
{
public:
	ColorBufferTestRenderData colorBuffer;

	virtual void setColor(const Color& color);
	virtual void render(RenderInfo& info);
};

class ColorBufferTestMeshActor : public Actor
{
public:
	Serialize(ColorBufferTestMeshActor, Actor);

	ColorBufferTestMeshRender colorBufferTestMeshRender;

	ColorBufferTestMeshActor(const string& name = "ColorBufferTestMeshActor");

	virtual void setMesh(Mesh* mesh);

	virtual void tick(float deltaTime);

	virtual void prerender(SceneRenderData& sceneData);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
};