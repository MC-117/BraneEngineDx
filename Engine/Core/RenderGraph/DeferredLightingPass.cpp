#include "DeferredLightingPass.h"
#include "../Camera.h"
#include "../RenderCore/RenderCommandList.h"
#include "../Console.h"
#include "../Profile/RenderProfile.h"
#include "../Asset.h"
#include "../RenderCore/RenderCoreUtility.h"
#include "../../Core/Utility/RenderUtility.h"
#include "Core/RenderCore/SurfaceBufferGetter.h"

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
					return t0.materialVariant < t1.materialVariant;
			}
		}
	}
	return false;
}

bool DeferredLightingTask::ExecutionOrder::operator()(const DeferredLightingTask* t0, const DeferredLightingTask* t1) const
{
	return (*this)(*t0, *t1);
}

ShaderProgram* DeferredLightingPass::blitProgram = NULL;
ShaderStage* DeferredLightingPass::blitFragmentShader = NULL;
GraphicsPipelineState* DeferredLightingPass::blitPSO = NULL;

bool DeferredLightingPass::loadDefaultResource()
{
	if (blitProgram)
		return true;
	IVendor& vendor = VendorManager::getInstance().getVendor();
	blitProgram = vendor.newShaderProgram();
	static string name = "LightingBlit";
	blitFragmentShader = vendor.newShaderStage({ Fragment_Shader_Stage, Shader_Default, name });
	const char* blitCode = "\
		struct ScreenVertexOut													\n\
		{																		\n\
			float4 svPos : SV_POSITION;											\n\
			float2 UV : TEXCOORD;												\n\
		};																		\n\
		struct FragmentOut														\n\
		{																		\n\
			float4 color : SV_Target;											\n\
			float depth : SV_Depth;												\n\
		};																		\n\
		Texture2D gBufferA : register(t7);										\n\
		SamplerState gBufferASampler : register(s7);							\n\
		Texture2D sceneDepthMap : register(t8);									\n\
		SamplerState sceneDepthMapSampler : register(s8);						\n\
		FragmentOut main(ScreenVertexOut fin)									\n\
		{																		\n\
			FragmentOut fout;													\n\
			fout.color = gBufferA.Sample(gBufferASampler, fin.UV);				\n\
			fout.depth = sceneDepthMap.Sample(sceneDepthMapSampler, fin.UV);	\n\
			return fout;														\n\
		}";
	string error;
	if (blitFragmentShader->compile(ShaderMacroSet(), blitCode, error) == 0) {
		Console::error("Compile Blit shader error: %s", error.c_str());
		throw runtime_error(error);
	}
	blitProgram->setMeshStage(*ShaderManager::getScreenVertexShader());
	blitProgram->addShaderStage(*blitFragmentShader);
	return blitProgram;
}

bool DeferredLightingPass::addTask(DeferredLightingTask& task)
{
	auto iter = lightingTask.find(&task);
	DeferredLightingTask* pTask;
	if (iter == lightingTask.end()) {
		pTask = new DeferredLightingTask(task);
		lightingTask.insert(pTask);
	}
	else {
		pTask = *iter;
	}
	pTask->age = 0;
	return true;
}

void DeferredLightingPass::prepare()
{
	blitProgram->init();
}

void DeferredLightingPass::execute(IRenderContext& context)
{
	context.clearFrameBindings();
	const bool isDebugView = VirtualShadowMapConfig::instance().debugViewMode == VirtualShadowMapConfig::ClipmapLevel;
	if (isDebugView) {
		for (auto scene : renderGraph->sceneDatas) {
			for (auto camera : scene->cameraRenderDatas) {
				if (IGBufferGetter* gBuffer = dynamic_cast<IGBufferGetter*>(camera->surfaceBuffer)) {
					SurfaceData surface = camera->surface;
					surface.clearFlags = Clear_All;
					surface.bindAndClear(context);
					blitSceneColor(context, *surface.renderTarget, gBuffer->getGBufferA(), gBuffer->getDepthTexture());
				}
			}
		}
	}
	DeferredLightingTask taskContext;
	for (auto item : lightingTask) {
		DeferredLightingTask& task = *item;
		task.age++;
		if (task.age > 1 || isDebugView)
			continue;

		// RENDER_DESC_SCOPE(DrawLighting, "Material(%s)", AssetInfo::getPath(task.material).c_str());
		RENDER_DESC_SCOPE(context, DrawLighting, "Material");

		bool cameraDataSwitch = false;
		bool shaderSwitch = false;

		if (taskContext.surface.renderTarget != task.surface.renderTarget) {
			taskContext.surface = task.surface;

			task.surface.bindAndClear(context);
			blitSceneColor(context, *task.surface.renderTarget, task.gBufferRT->getTexture(0), task.gBufferRT->getDepthTexture());
			taskContext.program = blitProgram;
		}

		if (taskContext.pipelineState != task.pipelineState) {
			taskContext.pipelineState = task.pipelineState;

			context.bindPipelineState(task.pipelineState);
			
			if (taskContext.program != task.program) {
				taskContext.program = task.program;

				shaderSwitch = true;
				task.sceneData->bind(context);
			}
		}

		if (taskContext.cameraRenderData != task.cameraRenderData || shaderSwitch) {
			taskContext.cameraRenderData = task.cameraRenderData;

			context.setViewport(0, 0, task.cameraRenderData->data.viewSize.x(), task.cameraRenderData->data.viewSize.y());
			task.cameraRenderData->bind(context);
		}

		if (taskContext.sceneData != task.sceneData || shaderSwitch) {
			taskContext.sceneData = task.sceneData;

			task.sceneData->bind(context);
			Texture2D* sceneDepth = task.gBufferRT->getDepthTexture();
			task.sceneData->virtualShadowMapRenderData.bindForLighting(context, sceneDepth);
		}
		
		if (taskContext.materialVariant != task.materialVariant) {
			static const ShaderPropertyName depthMapName = "depthMap";
			static const ShaderPropertyName gBufferAName = "gBufferA";
			static const ShaderPropertyName gBufferBName = "gBufferB";
			static const ShaderPropertyName gBufferCName = "gBufferC";
			static const ShaderPropertyName gBufferDName = "gBufferD";
			static const ShaderPropertyName gBufferEName = "gBufferE";
			static const ShaderPropertyName sceneDepthMapName = "sceneDepthMap";

			taskContext.materialVariant = task.materialVariant;
			bindMaterial(context, task.materialVariant);
			if (task.sceneData->lightDataPack.shadowTarget == NULL)
				context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), depthMapName);
			else
				context.bindTexture((ITexture*)task.sceneData->lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), depthMapName);

			Texture* gBufferA = task.gBufferRT->getTexture(0);
			Texture* gBufferB = task.gBufferRT->getTexture(1);
			Texture* gBufferC = task.gBufferRT->getTexture(2);
			Texture* gBufferD = task.gBufferRT->getTexture(3);
			Texture* gBufferE = task.gBufferRT->getTexture(4);
			Texture* sceneDepthMap = task.gBufferRT->getDepthTexture();

			context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferAName);
			context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBName);
			context.bindTexture((ITexture*)gBufferC->getVendorTexture(), gBufferCName);
			context.bindTexture((ITexture*)gBufferD->getVendorTexture(), gBufferDName);
			context.bindTexture((ITexture*)gBufferE->getVendorTexture(), gBufferEName);
			context.bindTexture((ITexture*)sceneDepthMap->getVendorTexture(), sceneDepthMapName);
		}

		/*if (taskContext.gBufferRT != task.gBufferRT) {
			taskContext.gBufferRT = task.gBufferRT;
		}*/
		
		context.setStencilRef(task.cameraRenderData->stencilRef);
		context.setDrawInfo(0, 0, task.materialVariant->desc.materialID);
		context.postProcessCall();
	}
	// context.setCullState(Cull_Back);
}

void DeferredLightingPass::reset()
{
	for (auto b = lightingTask.begin(), e = lightingTask.end(); b != e;) {
		DeferredLightingTask* task = *b;
		if (task->age > 2) {
			delete task;
			b = lightingTask.erase(b);
		}
		else b++;
	}
}

void DeferredLightingPass::blitSceneColor(IRenderContext& context, RenderTarget& target, Texture* gBufferA, Texture* sceneDepthMap)
{
	static const ShaderPropertyName gBufferAName = "gBufferA";
	static const ShaderPropertyName sceneDepthMapName = "sceneDepthMap";

	GraphicsPipelineStateDesc desc = GraphicsPipelineStateDesc::forScreen(
		blitProgram, &target, BM_Default);
	desc.renderMode = RenderMode(RS_Opaque);
	desc.renderMode.setDepthStencilMode(DepthStencilMode::DepthTestWritable());
	blitPSO = fetchPSOIfDescChangedThenInit(blitPSO, desc);
	
	context.bindPipelineState(blitPSO);
	// context.bindShaderProgram(blitProgram);
	context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferAName);
	context.bindTexture((ITexture*)sceneDepthMap->getVendorTexture(), sceneDepthMapName);
	context.setStencilRef(0);
	context.setViewport(0, 0, gBufferA->getWidth(), gBufferA->getHeight());
	context.postProcessCall();
}
