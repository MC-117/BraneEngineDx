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

struct ParticleRenderPack : public IRenderPack
{
	ParticleData* particleData;
	DrawArraysIndirectCommand cmd;
	GPUBuffer cmdBuffer = GPUBuffer(GB_Command, sizeof(DrawElementsIndirectCommand));

	virtual void excute(IRenderContext& context);
};