#include "ParticleRenderPack.h"
#include "RenderCommandList.h"
#include "RenderCoreUtility.h"

ParticleData* ParticleRenderData::setParticles(IRenderData* materialRenderData, const list<Particle>& particles)
{
	auto miter = this->particles.find(materialRenderData);
	ParticleData* pd;
	if (miter == this->particles.end()) {
		pd = &this->particles.emplace(pair<IRenderData*, ParticleData>(materialRenderData, ParticleData())).first->second;
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

ParticleData* ParticleRenderData::setParticles(IRenderData* materialRenderData, const vector<Particle>& particles)
{
	auto miter = this->particles.find(materialRenderData);
	ParticleData* pd;
	if (miter == this->particles.end()) {
		pd = &this->particles.emplace(pair<IRenderData*, ParticleData>(materialRenderData, ParticleData())).first->second;
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
	return sceneData && materialRenderData && materialRenderData->isValid() && particles && (mesh == NULL || (mesh && mesh->isValid()));
}

Enum<ShaderFeature> ParticleRenderCommand::getShaderFeature() const
{
	Enum<ShaderFeature> shaderFeature;
	shaderFeature |= Shader_Particle;
	return shaderFeature;
}

RenderMode ParticleRenderCommand::getRenderMode(const Name& passName, const CameraRenderData* cameraRenderData) const
{
	const int renderOrder = materialRenderData->renderOrder;
	RenderMode renderMode = RenderMode(renderOrder);
	renderMode.dsMode.stencilTest = materialRenderData->desc.enableStencilTest;

	bool cameraForceStencilTest = cameraRenderData->forceStencilTest && (passName == "Geometry"_N || passName == "Translucent"_N);
	
	if (cameraForceStencilTest) {
		renderMode.dsMode.stencilTest = true;
		renderMode.dsMode.stencilComparion_front = cameraRenderData->stencilCompare;
		renderMode.dsMode.stencilComparion_back = cameraRenderData->stencilCompare;
	}
	else {
		renderMode.dsMode.stencilComparion_front = materialRenderData->desc.stencilCompare;
		renderMode.dsMode.stencilComparion_back = materialRenderData->desc.stencilCompare;
	}
	return renderMode;
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

bool ParticleRenderPack::setRenderCommand(const IRenderCommand& command, const RenderTask& task)
{
	const ParticleRenderCommand* particleRenderCommand = dynamic_cast<const ParticleRenderCommand*>(&command);
	materialData = command.materialRenderData;
	particleData = particleRenderData.setParticles(materialData, *particleRenderCommand->particles);
	return particleData;
}

void ParticleRenderPack::excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext)
{
	if (particleData == NULL || particleData->batchCount == 0)
		return;

	if (taskContext.materialVariant != task.materialVariant) {
		taskContext.materialVariant = task.materialVariant;
		
		bindMaterial(context, task.materialVariant);
	}

	cmd.first = 0;
	cmd.count = 1;
	cmd.baseInstance = particleData->particleBase;
	cmd.instanceCount = particleData->batchCount;
	context.execteParticleDraw(NULL, { cmd });
}

void ParticleRenderPack::reset()
{
	materialData = NULL;
	particleData = NULL;
}
