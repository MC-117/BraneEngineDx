#include "BuildProbeGridPass.h"
#include "../RenderCore/SceneRenderData.h"
#include "../Asset.h"
#include "../Utility/MathUtility.h"
#include "../RenderCore/RenderCoreUtility.h"

bool BuildProbeGridPass::loadDefaultResource()
{
	if (buildMaterial == NULL)
		buildMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/BuildProbeGridLinkList.mat");
	if (compactMaterial == NULL)
		compactMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/CompactProbeGrid.mat");
	return buildMaterial && compactMaterial;
}

void BuildProbeGridPass::prepare()
{
	if (buildMaterial) {
		buildProgram = buildMaterial->getShader()->getProgram(Shader_Default);
		buildPSO = fetchPSOIfDescChangedThenInit(buildPSO, buildProgram);
		buildDebugProgram = buildMaterial->getShader()->getProgram(Shader_Debug);
		buildDebugPSO = fetchPSOIfDescChangedThenInit(buildDebugPSO, buildDebugProgram);
	}
	
	if (compactMaterial) {
		compactProgram = compactMaterial->getShader()->getProgram(Shader_Default);
		compactPSO = fetchPSOIfDescChangedThenInit(compactPSO, compactProgram);
	}
}

void BuildProbeGridPass::execute(IRenderContext& context)
{
	if (renderGraph == NULL || buildProgram == NULL || compactProgram == NULL)
		return;
	for (auto sceneData : renderGraph->sceneDatas) {
		for (auto cameraData : sceneData->cameraRenderDatas) {
			ProbeGridRenderData& probeGrid = cameraData->probeGrid;

			// Build link list
			Vector3u localSize = buildMaterial->getLocalSize();
			Vector3u gridSize = cameraData->probeGridInfo.probeGridSize;
			Vector3u dispatchDim = Vector3u(
				ceil(gridSize.x() / (float)localSize.x()),
				ceil(gridSize.y() / (float)localSize.y()),
				ceil(gridSize.z() / (float)localSize.z()));

			int probeGridCellCount = cameraData->probeGridInfo.probeGridCellCount;
			int probeMaxSlotCount = cameraData->probeGridInfo.probeMaxLinkListLength;

			probeGrid.probeGridLinkHeadBuffer.resize(probeGridCellCount);
			probeGrid.probeGridCulledHeadBuffer.resize(probeGridCellCount);

			probeGrid.probeGridLinkListBuffer.resize(probeMaxSlotCount + 1);
			probeGrid.probeGridCulledListBuffer.resize(probeMaxSlotCount + 1);

			Vector4u initData = Vector4u::Zero();
			context.clearOutputBufferUint(probeGrid.probeGridLinkListBuffer.getVendorGPUBuffer(), initData);
			context.clearOutputBufferUint(probeGrid.probeGridCulledListBuffer.getVendorGPUBuffer(), initData);

			if (cameraData->flags.has(CameraRender_DebugDraw) &&
				(sceneData->debugRenderData.isTriggerPersistentDebugDraw() ||
					cameraData->getDebugProbeIndex() > 0))
				context.bindPipelineState(buildDebugPSO);
			else
				context.bindPipelineState(buildPSO);
			sceneData->debugRenderData.bind(context);
			cameraData->bindCameraBuffOnly(context);
			cameraData->probeGrid.probePool->bind(context);
			context.bindBufferBase(probeGrid.probeGridLinkListBuffer.getVendorGPUBuffer(), probeGrid.outProbeGridLinkListName, { true });
			context.bindBufferBase(probeGrid.probeGridLinkHeadBuffer.getVendorGPUBuffer(), probeGrid.outProbeGridLinkHeadName, { true });
			context.dispatchCompute(dispatchDim.x(), dispatchDim.y(), dispatchDim.z());

			context.unbindBufferBase(probeGrid.outProbeGridLinkListName);
			context.unbindBufferBase(probeGrid.outProbeGridLinkHeadName);
			sceneData->debugRenderData.unbind(context);

			// Compact link list to array
			context.bindPipelineState(compactPSO);
			cameraData->bindCameraBuffOnly(context);
			context.bindBufferBase(probeGrid.probeGridLinkListBuffer.getVendorGPUBuffer(), probeGrid.probeGridLinkListName);
			context.bindBufferBase(probeGrid.probeGridLinkHeadBuffer.getVendorGPUBuffer(), probeGrid.probeGridLinkHeadName);
			context.bindBufferBase(probeGrid.probeGridCulledListBuffer.getVendorGPUBuffer(), probeGrid.outProbeGridCulledListName, { true });
			context.bindBufferBase(probeGrid.probeGridCulledHeadBuffer.getVendorGPUBuffer(), probeGrid.outProbeGridCulledHeadName, { true });
			context.dispatchCompute(dispatchDim.x(), dispatchDim.y(), dispatchDim.z());

			context.unbindBufferBase(probeGrid.outProbeGridCulledListName);
			context.unbindBufferBase(probeGrid.outProbeGridCulledHeadName);
		}
	}
}

void BuildProbeGridPass::reset()
{
}
