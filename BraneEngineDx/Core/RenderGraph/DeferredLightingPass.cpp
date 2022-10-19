#include "DeferredLightingPass.h"
#include "../Camera.h"
#include "../RenderCore/RenderCommandList.h"
#include "../Console.h"

bool DeferredLightingTask::ExecutionOrder::operator()(const DeferredLightingTask& t0, const DeferredLightingTask& t1) const
{
	if (t0.sceneData < t1.sceneData)
		return true;
	if (t0.sceneData == t1.sceneData) {
		if (t0.program < t1.program)
			return true;
		if (t0.program == t1.program) {
			if (t0.gBufferRT < t1.gBufferRT)
				return true;
			if (t0.gBufferRT == t1.gBufferRT) {
				if (t0.cameraRenderData < t1.cameraRenderData)
					return true;
				if (t0.cameraRenderData == t1.cameraRenderData)
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
	materialData->create();
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

		if (taskContext.cameraRenderData != task.cameraRenderData) {
			taskContext.cameraRenderData = task.cameraRenderData;

			task.cameraRenderData->upload();
			task.cameraRenderData->bind(context);

			context.bindFrame(task.cameraRenderData->renderTarget->getVendorRenderTarget());
			context.clearFrameColors(task.cameraRenderData->clearColors);
			context.clearFrameDepth(1);
			blitSceneColor(context, task.gBufferRT->getTexture(0), task.gBufferRT->getTexture(1));
			taskContext.program = blitProgram;
		}

		if (taskContext.sceneData != task.sceneData) {
			taskContext.sceneData = task.sceneData;

			task.sceneData->bind(context);
		}

		if (taskContext.program != task.program) {
			taskContext.program = task.program;

			context.bindShaderProgram(task.program);
		}
		
		if (taskContext.materialRenderData != task.materialRenderData) {
			taskContext.materialRenderData = task.materialRenderData;
			task.materialRenderData->upload();
			task.materialRenderData->bind(context);
			if (task.sceneData->lightDataPack.shadowTarget == NULL)
				context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), "depthMap");
			else
				context.bindTexture((ITexture*)task.sceneData->lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), "depthMap");

			Texture* gBufferA = task.gBufferRT->getTexture(0);
			Texture* gBufferB = task.gBufferRT->getTexture(1);
			Texture* gBufferC = task.gBufferRT->getTexture(2);
			Texture* gBufferD = task.gBufferRT->getTexture(3);
			Texture* gBufferE = task.gBufferRT->getTexture(4);

			context.bindTexture((ITexture*)gBufferA->getVendorTexture(), "gBufferA");
			context.bindTexture((ITexture*)gBufferB->getVendorTexture(), "gBufferB");
			context.bindTexture((ITexture*)gBufferC->getVendorTexture(), "gBufferC");
			context.bindTexture((ITexture*)gBufferD->getVendorTexture(), "gBufferD");
			context.bindTexture((ITexture*)gBufferE->getVendorTexture(), "gBufferE");
		}

		/*if (taskContext.gBufferRT != task.gBufferRT) {
			taskContext.gBufferRT = task.gBufferRT;
		}*/

		context.setRenderPostState();
		context.setDrawInfo(0, 0, task.materialRenderData->desc.materialID);
		context.postProcessCall();
	}
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
	if (blitFragmentShader->compile(blitCode, error) == 0) {
		Console::error("Compile Blit shader error: %s", error.c_str());
		throw runtime_error(error);
	}
	blitProgram->addShaderStage(*ShaderManager::getScreenVertexShader());
	blitProgram->addShaderStage(*blitFragmentShader);
	isInit = true;
}

void DeferredLightingPass::blitSceneColor(IRenderContext& context, Texture* gBufferA, Texture* gBufferB)
{
	context.bindShaderProgram(blitProgram);
	context.bindTexture((ITexture*)gBufferA->getVendorTexture(), "gBufferA");
	context.bindTexture((ITexture*)gBufferB->getVendorTexture(), "gBufferB");
	context.setRenderOpaqueState();
	context.postProcessCall();
}
