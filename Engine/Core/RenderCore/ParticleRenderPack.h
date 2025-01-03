#pragma once

#include "RenderInterface.h"
#include "../GPUBuffer.h"
#include "../Material.h"

struct ParticleData
{
	vector<Particle> particles;
	unsigned int batchCount = 0;
	unsigned int particleBase = 0;
};

struct ENGINE_API ParticleRenderData : public IRenderData
{
	unsigned int totalParticleCount = 0;
	map<IRenderData*, ParticleData> particles;
	GPUBuffer particleBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(Particle));

	ParticleData* setParticles(IRenderData* materialRenderData, const list<Particle>& particles);
	ParticleData* setParticles(IRenderData* materialRenderData, const vector<Particle>& particles);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};

struct ENGINE_API ParticleRenderCommand : public IRenderCommand
{
	list<Particle>* particles;
	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual RenderMode getRenderMode(const Name& passName, const CameraRenderData* cameraRenderData) const;
	virtual bool canCastShadow() const;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

struct ParticleRenderPack : public IRenderPack
{
	ParticleRenderData& particleRenderData;

	MaterialRenderData* materialData = NULL;
	ParticleData* particleData = NULL;
	DrawArraysIndirectCommand cmd;
	GPUBuffer cmdBuffer = GPUBuffer(GB_Command, GBF_Struct, sizeof(DrawElementsIndirectCommand));

	ParticleRenderPack(ParticleRenderData& particleRenderData);

	virtual bool setRenderCommand(const IRenderCommand& command);
	virtual void excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext);
	virtual void reset();
};