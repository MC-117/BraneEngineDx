#include "CMAA2Pass.h"
#include "../Asset.h"
#include "../Console.h"

CMAA2Pass::CMAA2Pass(const string& name, Material* material) : PostProcessPass(name, material)
{
	workingEdgesTexture.setAutoGenMip(false);
	workingDeferredBlendItemListHeadsTexture.setAutoGenMip(false);
}

void CMAA2Pass::prepare()
{
}

void CMAA2Pass::execute(IRenderContext& context)
{
	// first pass edge detect
	context.bindShaderProgram(edgesColor2x2Program);

	Image image;
	image.texture = &workingEdgesTexture;
	context.bindImage(image, 1);
	context.bindBufferBase(workingShapeCandidatesBuffer.getVendorGPUBuffer(), 2, { true });
	context.bindBufferBase(workingDeferredBlendLocationListBuffer.getVendorGPUBuffer(), 3, { true });
	context.bindBufferBase(workingDeferredBlendItemListBuffer.getVendorGPUBuffer(), 4, { true });
	image.texture = &workingDeferredBlendItemListHeadsTexture;
	context.bindImage(image, 5);
	context.bindBufferBase(workingControlBuffer.getVendorGPUBuffer(), 6, { true });
	context.bindBufferBase(workingExecuteIndirectBuffer.getVendorGPUBuffer(), 7, { true });

	context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), Compute_Shader_Stage, 0, 0);

	int csOutputKernelSizeX = edgesColor2x2Material->getLocalSize().x() - 2;
	int csOutputKernelSizeY = edgesColor2x2Material->getLocalSize().y() - 2;
	int threadGroupCountX = (size.x + csOutputKernelSizeX * 2 - 1) / (csOutputKernelSizeX * 2);
	int threadGroupCountY = (size.y + csOutputKernelSizeY * 2 - 1) / (csOutputKernelSizeY * 2);

	context.dispatchCompute(threadGroupCountX, threadGroupCountY, 1);

	// Set up for the first DispatchIndirect
	context.bindShaderProgram(computeDispatchArgsProgram);
	context.dispatchCompute(2, 1, 1);

	// Process shape candidates DispatchIndirect
	context.bindShaderProgram(processCandidatesProgram);
	context.dispatchComputeIndirect(workingExecuteIndirectBuffer.getVendorGPUBuffer(), 0);

	// Set up for the second DispatchIndirect
	context.bindShaderProgram(computeDispatchArgsProgram);
	context.dispatchCompute(1, 2, 1);

	// Resolve & apply blended colors
	context.bindShaderProgram(deferredColorApply2x2Program);

	context.bindTexture(NULL, Compute_Shader_Stage, 0, 0);
	image.texture = resource->screenTexture;
	context.bindImage(image, 0);

	context.dispatchComputeIndirect(workingExecuteIndirectBuffer.getVendorGPUBuffer(), 0);
	context.setGPUSignal();
	context.waitSignalGPU();

	if (enableDebugEdges) {
		// DEBUGGING
		int tgcX = (size.x + 16 - 1) / 16;
		int tgcY = (size.y + 16 - 1) / 16;

		context.bindShaderProgram(debugDrawEdgesProgram);
		context.dispatchCompute(tgcX, tgcY, 1);
	}

	context.clearOutputBufferBindings();
}

bool CMAA2Pass::mapMaterialParameter(RenderInfo& info)
{
	if (edgesColor2x2Material == NULL)
		edgesColor2x2Material = getAssetByPath<Material>("Engine/Shaders/PostProcess/CMAA2/CMAA2_EdgesColor2x2.mat");
	if (processCandidatesMaterial == NULL)
		processCandidatesMaterial = getAssetByPath<Material>("Engine/Shaders/PostProcess/CMAA2/CMAA2_ProcessCandidates.mat");
	if (computeDispatchArgsMaterial == NULL)
		computeDispatchArgsMaterial = getAssetByPath<Material>("Engine/Shaders/PostProcess/CMAA2/CMAA2_ComputeDispatchArgs.mat");
	if (deferredColorApply2x2Material == NULL)
		deferredColorApply2x2Material = getAssetByPath<Material>("Engine/Shaders/PostProcess/CMAA2/CMAA2_DeferredColorApply2x2.mat");
	if (debugDrawEdgesMaterial == NULL)
		debugDrawEdgesMaterial = getAssetByPath<Material>("Engine/Shaders/PostProcess/CMAA2/CMAA2_DebugDrawEdges.mat");

	if (edgesColor2x2Material == NULL || processCandidatesMaterial == NULL ||
		computeDispatchArgsMaterial == NULL || deferredColorApply2x2Material == NULL ||
		debugDrawEdgesMaterial == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	return true;
}

void CMAA2Pass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;

	edgesColor2x2Program = edgesColor2x2Material->getShader()->getProgram(Shader_Default);
	processCandidatesProgram = processCandidatesMaterial->getShader()->getProgram(Shader_Default);
	computeDispatchArgsProgram = computeDispatchArgsMaterial->getShader()->getProgram(Shader_Default);
	deferredColorApply2x2Program = deferredColorApply2x2Material->getShader()->getProgram(Shader_Default);
	debugDrawEdgesProgram = debugDrawEdgesMaterial->getShader()->getProgram(Shader_Default);

	if (edgesColor2x2Program == NULL || processCandidatesProgram == NULL ||
		computeDispatchArgsProgram == NULL || deferredColorApply2x2Program == NULL ||
		debugDrawEdgesProgram == NULL) {
		Console::error("PostProcessPass: Shader_Default not found in CMAA2 shaders");
		return;
	}

	if (edgesColor2x2Program->isComputable() &&
		processCandidatesProgram->isComputable() &&
		computeDispatchArgsProgram->isComputable() &&
		deferredColorApply2x2Program->isComputable() &&
		debugDrawEdgesProgram->isComputable()) {

		edgesColor2x2Program->init();
		processCandidatesProgram->init();
		computeDispatchArgsProgram->init();
		deferredColorApply2x2Program->init();
		debugDrawEdgesProgram->init();

		info.renderGraph->addPass(*this);
	}
}

void CMAA2Pass::onGUI(EditorInfo& info)
{
	ImGui::Checkbox("EnableDebugEdges", &enableDebugEdges);
}

void CMAA2Pass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);

	const int m_textureSampleCount = 1;
	const int requiredCandidatePixels = size.x * size.y / 4 * m_textureSampleCount;
	const int requiredDeferredColorApplyBuffer = size.x * size.y / 2 * m_textureSampleCount;
	const int requiredListHeadsPixels = (size.x * size.y + 3) / 6;

	workingEdgesTexture.resize((size.x + 1) / 2, size.y);
	workingDeferredBlendItemListHeadsTexture.resize((size.x + 1) / 2, (size.y + 1) / 2);

	workingShapeCandidatesBuffer.resize(requiredCandidatePixels);
	workingDeferredBlendItemListBuffer.resize(requiredDeferredColorApplyBuffer);
	workingDeferredBlendLocationListBuffer.resize(requiredListHeadsPixels);

	unsigned int initData[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	workingControlBuffer.uploadData(16, initData);
	workingExecuteIndirectBuffer.resize(4);
}
