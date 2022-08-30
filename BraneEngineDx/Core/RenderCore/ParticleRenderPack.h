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

struct ParticleRenderData : public IRenderData
{
	unsigned int totalParticleCount = 0;
	map<Material*, ParticleData> particles;
	GPUBuffer particleBuffer = GPUBuffer(GB_Struct, sizeof(Particle));

	ParticleData* setParticles(Material* material, const list<Particle>& particles);
	ParticleData* setParticles(Material* material, const vector<Particle>& particles);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};

struct ParticleRenderCommand : public IRenderCommand
{
	list<Particle>* particles;
	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual RenderMode getRenderMode() const;
	virtual IRenderPack* createRenderPack(RenderCommandList& commandList) const;
};

struct ParticleRenderPack : public IRenderPack
{
	ParticleRenderData& particleRenderData;

	MaterialRenderData* materialData = NULL;
	ParticleData* particleData = NULL;
	DrawArraysIndirectCommand cmd;
	GPUBuffer cmdBuffer = GPUBuffer(GB_Command, sizeof(DrawElementsIndirectCommand));

	ParticleRenderPack(ParticleRenderData& particleRenderData);

	virtual bool setRenderCommand(const IRenderCommand& command);
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext);
};