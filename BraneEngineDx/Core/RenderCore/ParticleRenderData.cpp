#include "ParticleRenderData.h"

ParticleData* ParticleRenderData::setParticles(Material* material, const list<Particle>& particles)
{
	auto miter = this->particles.find(material);
	ParticleData* pd;
	if (miter == this->particles.end()) {
		pd = &this->particles.emplace(pair<Material*, ParticleData>(material, ParticleData())).first->second;
	}
	else {
		pd = &miter->second;
	}
	int base = pd->batchCount;
	totalParticleCount += particles.size();
	pd->batchCount += particles.size();
	if (pd->batchCount > pd->particles.size())
		pd->particles.resize(pd->batchCount);
	int i = 0;
	for (auto b = particles.begin(), e = particles.end(); b != e; b++, i++) {
		pd->particles[base + i] = *b;
	}
	return pd;
}

ParticleData* ParticleRenderData::setParticles(Material* material, const vector<Particle>& particles)
{
	auto miter = this->particles.find(material);
	ParticleData* pd;
	if (miter == this->particles.end()) {
		pd = &this->particles.emplace(pair<Material*, ParticleData>(material, ParticleData())).first->second;
	}
	else {
		pd = &miter->second;
	}
	int base = pd->batchCount;
	totalParticleCount += particles.size();
	pd->batchCount += particles.size();
	if (pd->batchCount > pd->particles.size())
		pd->particles.resize(pd->batchCount);
	int i = 0;
	for (auto b = particles.begin(), e = particles.end(); b != e; b++, i++) {
		pd->particles[base + i] = *b;
	}
	return pd;
}

void ParticleRenderData::create()
{
	int base = 0;
	for (auto b = particles.begin(), e = particles.end(); b != e; b++) {
		b->second.particleBase = base;
		base += b->second.batchCount;
	}
}

void ParticleRenderData::release()
{
}

void ParticleRenderData::upload()
{
	particleBuffer.resize(totalParticleCount);
	for (auto b = particles.begin(), e = particles.end(); b != e; b++) {
		particleBuffer.uploadSubData(b->second.particleBase,
			b->second.batchCount, b->second.particles.data()->position.data());
	}
}

void ParticleRenderData::bind(IRenderContext& context)
{
	context.bindBufferBase(particleBuffer.getVendorGPUBuffer(), PARTICLE_BIND_INDEX);
}

void ParticleRenderData::clean()
{
	for (auto b = particles.begin(), e = particles.end(); b != e; b++) {
		b->second.batchCount = 0;
	}
	totalParticleCount = 0;
}

void ParticleRenderPack::excute(IRenderContext& context)
{
	if (particleData == NULL)
		return;
	cmd.first = 0;
	cmd.count = 1;
	cmd.baseInstance = particleData->particleBase;
	cmd.instanceCount = particleData->batchCount;
	newVendorRenderExecution();
	context.execteParticleDraw(vendorRenderExecution, { cmd });
}
