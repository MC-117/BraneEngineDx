#include "DeferredLightingPass.h"
#include "../Camera.h"
#include "../RenderCore/RenderCommandList.h"
#include "../Console.h"

bool DeferredLightingTask::ExecutionOrder::operator()(const DeferredLightingTask& t0, const DeferredLightingTask& t1) const
{
	if (t0.sceneData < t1.sceneData)
		return true;
	if (t0.sceneData == t1.sceneData) {
		if (t0.surface.renderTarget < t1.surface.renderTarget)
			return true;
		if (t0.surface.renderTarget == t1.surface.renderTarget) {
			if (t0.cameraRenderData < t1.cameraRenderData)
				return true;
			if (t0.cameraRenderData == t1.cameraRenderData) {
				if (t0.program < t1.program)
					return true;
				if (t0.program == t1.program)
					return t0.material < t1.material;
			}
		}
	}
	return false;
}

bool DeferredLightingTask::ExecutionOrder::operator()(const DeferredLightingTask* t0, const DeferredLightingTask* t1) const
{
	return (*this)(*t0, *t1);
}

bool DeferredLightingPass::isInit = false;
ShaderProgram* DeferredLightingPass::blitProgram = NULL;
ShaderStage* DeferredLightingPass::blitFragmentShader = NULL;

bool DeferredLightingPass::addTask(DeferredLightingTask& task)
{
	auto iter = lightingTask.find(&task);
	MaterialRenderData* materialData = NULL;
	DeferredLightingTask* pTask;
	if (iter == lightingTask.end()) {
		materialData = new MaterialRenderData();
		materialData->material = task.material;
		materialData->program = task.program;
		task.materialRenderData = materialData;
		pTask = new DeferredLightingTask(task);
		lightingTask.insert(pTask);
		materialRenderDatas.push_back(materialData);
	}
	else {
		pTask = *iter;
		materialData = pTask->materialRenderData;
	}
	if (materialData->usedFrame < (long long)Time::frames()) {
		materialData->create();
		renderGraph->getRenderDataCollector()->add(*materialData);
		materialData->usedFrame = Time::frames();
	}
	pTask->age = 0;
	return true;
}

void DeferredLightingPass::prepare()
{
	LoadDefaultShader();
}

void DeferredLightingPass::execute(IRenderContext& context)
{
	context.clearFrameBindings();
	DeferredLightingTask taskContext;
	for (auto item : lightingTask) {
		DeferredLightingTask& task = *item;
		task.age++;
		if (task.age > 1)
			continue;

		bool cameraDataSwitch = false;
		bool shaderSwitch = false;

		if (taskContext.surface.renderTarget != task.surface.renderTarget) {
			taskContext.surface = task.surface;

			task.surface.bind(context);
			blitSceneColor(context, task.gBufferRT->getTexture(0), task.gBufferRT->getTexture(1));
			taskContext.program = blitProgram;
		}

		if (taskContext.program != task.program) {
			taskContext.program = task.program;

			context.bindShaderProgram(task.program);

			shaderSwitch = true;
			task.sceneData->bind(context);
		}

		if (taskContext.cameraRenderData != task.cameraRenderData || shaderSwitch) {
			taskContext.cameraRenderData = task.cameraRenderData;

			context.setViewport(0, 0, task.cameraRenderData->data.viewSize.x(), task.cameraRenderData->data.viewSize.y());
			task.cameraRenderData->bind(context);
		}

		if (taskContext.sceneData != task.sceneData || shaderSwitch) {
			taskContext.sceneData = task.sceneData;

			task.sceneData->bind(context);
			task.sceneData->virtualShadowMapRenderData.bindForLighting(context);
		}
		
		if (taskContext.materialRenderData != task.materialRenderData) {
			static const ShaderPropertyName depthMapName = "depthMap";
			static const ShaderPropertyName gBufferAName = "gBufferA";
			static const ShaderPropertyName gBufferBName = "gBufferB";
			static const ShaderPropertyName gBufferCName = "gBufferC";
			static const ShaderPropertyName gBufferDName = "gBufferD";
			static const ShaderPropertyName gBufferEName = "gBufferE";
			static const ShaderPropertyName gBufferFName = "gBufferF";

			taskContext.materialRenderData = task.materialRenderData;
			task.materialRenderData->bindCullMode(context, false);
			task.materialRenderData->bind(context);
			if (task.sceneData->lightDataPack.shadowTarget == NULL)
				context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), depthMapName);
			else
				context.bindTexture((ITexture*)task.sceneData->lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), depthMapName);

			Texture* gBufferA = task.gBufferRT->getTexture(0);
			Texture* gBufferB = task.gBufferRT->getTexture(1);
			Texture* gBufferC = task.gBufferRT->getTexture(2);
			Texture* gBufferD = task.gBufferRT->getTexture(3);
			Texture* gBufferE = task.gBufferRT->getTexture(4);
			Texture* gBufferF = task.gBufferRT->getTexture(5);

			context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferAName);
			context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBName);
			context.bindTexture((ITexture*)gBufferC->getVendorTexture(), gBufferCName);
			context.bindTexture((ITexture*)gBufferD->getVendorTexture(), gBufferDName);
			context.bindTexture((ITexture*)gBufferE->getVendorTexture(), gBufferEName);
			context.bindTexture((ITexture*)gBufferF->getVendorTexture(), gBufferFName);
		}

		/*if (taskContext.gBufferRT != task.gBufferRT) {
			taskContext.gBufferRT = task.gBufferRT;
		}*/

		context.setRenderPostState();
		context.setDrawInfo(0, 0, task.materialRenderData->desc.materialID);
		context.postProcessCall();
	}
	context.setCullState(Cull_Back);
}

void DeferredLightingPass::reset()
{
	for (auto b = lightingTask.begin(), e = lightingTask.end(); b != e;) {
		DeferredLightingTask* task = *b;
		if (task->age > 2) {
			task->materialRenderData->release();
			delete task->materialRenderData;
			delete task;
			b = lightingTask.erase(b);
		}
		else b++;
	}
}

void DeferredLightingPass::LoadDefaultShader()
{
	if (isInit)
		return;
	IVendor& vendor = VendorManager::getInstance().getVendor();
	blitProgram = vendor.newShaderProgram();
	static string name = "LightingBlit";
	blitFragmentShader = vendor.newShaderStage({ Fragment_Shader_Stage, Shader_Default, name });
	const char* blitCode = "\
		struct ScreenVertexOut										\n\
		{															\n\
			float4 svPos : SV_POSITION;								\n\
			float2 UV : TEXCOORD;									\n\
		};															\n\
		struct FragmentOut											\n\
		{															\n\
			float4 color : SV_Target;								\n\
			float depth : SV_Depth;									\n\
		};															\n\
		Texture2D gBufferA : register(t7);							\n\
		SamplerState gBufferASampler : register(s7);				\n\
		Texture2D gBufferB : register(t8);							\n\
		SamplerState gBufferBSampler : register(s8);				\n\
		FragmentOut main(ScreenVertexOut fin)						\n\
		{															\n\
			FragmentOut fout;										\n\
			fout.color = gBufferA.Sample(gBufferASampler, fin.UV);	\n\
			fout.depth = gBufferB.Sample(gBufferBSampler, fin.UV).r;\n\
			return fout;											\n\
		}";
	string error;
	if (blitFragmentShader->compile(ShaderMacroSet(), blitCode, error) == 0) {
		Console::error("Compile Blit shader error: %s", error.c_str());
		throw runtime_error(error);
	}
	blitProgram->setMeshStage(*ShaderManager::getScreenVertexShader());
	blitProgram->addShaderStage(*blitFragmentShader);
	isInit = true;
}

void DeferredLightingPass::blitSceneColor(IRenderContext& context, Texture* gBufferA, Texture* gBufferB)
{
	static const ShaderPropertyName gBufferAName = "gBufferA";
	static const ShaderPropertyName gBufferBName = "gBufferB";

	blitProgram->init();
	context.bindShaderProgram(blitProgram);
	context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferAName);
	context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBName);
	context.setRenderOpaqueState();
	context.setViewport(0, 0, gBufferA->getWidth(), gBufferA->getHeight());
	context.postProcessCall();
}