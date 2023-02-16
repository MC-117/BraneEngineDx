#include "ParticleRenderPack.h"
#include "RenderCommandList.h"
#include "../SkeletonMesh.h"

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
	static const ShaderPropertyName ParticlesName = "Particles";
	context.bindBufferBase(particleBuffer.getVendorGPUBuffer(), ParticlesName); // PARTICLE_BIND_INDEX
}

void ParticleRenderData::clean()
{
	for (auto b = particles.begin(), e = particles.end(); b != e; b++) {
		b->second.batchCount = 0;
	}
	totalParticleCount = 0;
}

bool ParticleRenderCommand::isValid() const
{
	return sceneData && material && !material->isNull() && particles && (mesh == NULL || (mesh && mesh->isValid()));
}

Enum<ShaderFeature> ParticleRenderCommand::getShaderFeature() const
{
	Enum<ShaderFeature> shaderFeature;
	shaderFeature |= Shader_Particle;
	return shaderFeature;
}

RenderMode ParticleRenderCommand::getRenderMode() const
{
	return RenderMode(material->getRenderOrder(), 0, 0);
}

bool ParticleRenderCommand::canCastShadow() const
{
	return false;
}

IRenderPack* ParticleRenderCommand::createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const
{
	return new ParticleRenderPack(sceneData.particleDataPack);
}

ParticleRenderPack::ParticleRenderPack(ParticleRenderData& particleRenderData)
	: particleRenderData(particleRenderData)
{
}

bool ParticleRenderPack::setRenderCommand(const IRenderCommand& command)
{
	const ParticleRenderCommand* particleRenderCommand = dynamic_cast<const ParticleRenderCommand*>(&command);
	materialData = dynamic_cast<MaterialRenderData*>(command.material->getRenderData());
	particleData = particleRenderData.setParticles(particleRenderCommand->material, *particleRenderCommand->particles);
	return particleData;
}

void ParticleRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	if (particleData == NULL || particleData->batchCount == 0)
		return;

	if (taskContext.materialData != materialData) {
		taskContext.materialData = materialData;
		materialData->bind(context);
	}

	cmd.first = 0;
	cmd.count = 1;
	cmd.baseInstance = particleData->particleBase;
	cmd.instanceCount = particleData->batchCount;
	newVendorRenderExecution();
	context.execteParticleDraw(vendorRenderExecution, { cmd });
}
