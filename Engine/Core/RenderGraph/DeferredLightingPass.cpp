#include "DeferredLightingPass.h"
#include "../Camera.h"
#include "../RenderCore/RenderCommandList.h"
#include "../Console.h"
#include "../Profile/RenderProfile.h"
#include "../Asset.h"
#include "../RenderCore/RenderCoreUtility.h"
#include "../../Core/Utility/RenderUtility.h"

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

bool DeferredLightingPass::loadDefaultResource()
{
	if (blitProgram)
		return true;
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
			fout.depth = gBufferB.Sample(gBufferBSampler, fin.UV);	\n\
			return fout;											\n\
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
	DeferredLightingTask taskContext;
	for (auto item : lightingTask) {
		DeferredLightingTask& task = *item;
		task.age++;
		if (task.age > 1)
			continue;

		// RENDER_DESC_SCOPE(DrawLighting, "Material(%s)", AssetInfo::getPath(task.material).c_str());
		RENDER_DESC_SCOPE(context, DrawLighting, "Material");

		bool cameraDataSwitch = false;
		bool shaderSwitch = false;

		if (taskContext.surface.renderTarget != task.surface.renderTarget) {
			taskContext.surface = task.surface;

			task.surface.bind(context);
			blitSceneColor(context, *task.surface.renderTarget, task.gBufferRT->getTexture(0), task.gBufferRT->getTexture(1));
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
		
		if (taskContext.materialVariant != task.materialVariant) {
			static const ShaderPropertyName depthMapName = "depthMap";
			static const ShaderPropertyName gBufferAName = "gBufferA";
			static const ShaderPropertyName gBufferBName = "gBufferB";
			static const ShaderPropertyName gBufferCName = "gBufferC";
			static const ShaderPropertyName gBufferDName = "gBufferD";
			static const ShaderPropertyName gBufferEName = "gBufferE";
			static const ShaderPropertyName gBufferFName = "gBufferF";

			taskContext.materialVariant = task.materialVariant;
			bindMaterialCullMode(context, task.materialVariant, false);
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

		DepthStencilMode depthStencilMode;
		setDepthStateFromRenderOrder(depthStencilMode, RS_Post);
		depthStencilMode.depthTest = task.cameraRenderData->forceStencilTest;
		depthStencilMode.stencilWriteMask = 0;
		depthStencilMode.stencilComparion_front = RCT_Equal;
		depthStencilMode.stencilComparion_back = RCT_Equal;

		context.setRenderPostState(depthStencilMode);
		context.setStencilRef(task.cameraRenderData->stencilRef);
		context.setDrawInfo(0, 0, task.materialVariant->desc.materialID);
		context.postProcessCall();
	}
	context.setCullState(Cull_Back);
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

void DeferredLightingPass::blitSceneColor(IRenderContext& context, RenderTarget& target, Texture* gBufferA, Texture* gBufferB)
{
	static const ShaderPropertyName gBufferAName = "gBufferA";
	static const ShaderPropertyName gBufferBName = "gBufferB";

	context.bindShaderProgram(blitProgram);
	context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferAName);
	context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBName);
	context.setRenderOpaqueState(DepthStencilMode::DepthTestWritable());
	context.setStencilRef(0);
	context.setViewport(0, 0, gBufferA->getWidth(), gBufferA->getHeight());
	context.postProcessCall();
}
