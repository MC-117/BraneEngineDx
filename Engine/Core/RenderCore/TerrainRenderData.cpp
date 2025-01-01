#include "TerrainRenderData.h"

#include "CameraRenderData.h"
#include "../Utility/RenderUtility.h"
#include "../Asset.h"
#include "../Material.h"
#include "Core/Profile/RenderProfile.h"

#undef min
#undef max

TerrainBatchDrawArray::TerrainBatchDrawArray()
	: selectedGrids(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, selectedTiles(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, selectTileArgs(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, drawArgs(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
	, needCalBounds(true)
	, needUpdate(true)
{
}

void TerrainBatchDrawArray::update(const TerrainCalTileBoundParameters& parameters)
{
	selectedGrids.resize(parameters.allGrids);
	selectedTiles.resize(parameters.tilesFromAllLevelPerGrid * parameters.allGrids);
	selectTileArgs.resize(4);
	drawArgs.resize(sizeof(DrawElementsIndirectCommand) / sizeof(unsigned int));
}

void TerrainBatchDrawArray::bindInstanceBuffer(IRenderContext& context)
{
}

unsigned TerrainBatchDrawArray::getInstanceCount() const
{
	return 1;
}

unsigned TerrainBatchDrawArray::getCommandCount() const
{
	return 1;
}

Material* TerrainQuadTree::calTileBoundsMaterial = NULL;
ShaderProgram* TerrainQuadTree::calTileBoundsProgram = NULL;
Material* TerrainQuadTree::calTileLodBoundsMaterial = NULL;
ShaderProgram* TerrainQuadTree::calTileLodBoundsProgram = NULL;
Material* TerrainQuadTree::selectGridsMaterial = NULL;
ShaderProgram* TerrainQuadTree::selectGridsProgram = NULL;
Material* TerrainQuadTree::selectTilesMaterial = NULL;
ShaderProgram* TerrainQuadTree::selectTilesProgram = NULL;
bool TerrainQuadTree::isInited = false;

TerrainQuadTree::TerrainQuadTree()
	: tileBounds(1, 1, 2, false,
		{ TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RG32_F })
	, parameterBuffer(GB_Constant, GBF_Struct, sizeof(TerrainCalTileBoundParameters))
{
}

void TerrainQuadTree::setTerrainData(const TerrainData& data, const Matrix4f& localToWorld, Texture2D* heightMap)
{
	needCalBounds = this->heightMap != heightMap
		|| parameters.widthPerGrid != data.widthPerGrid
		|| parameters.widthPerTriangle != data.widthPerTriangle
		|| parameters.tilesPerGrid != data.tilesPerGrid
		|| parameters.trianglesPerTile != data.trianglesPerTile
		|| parameters.tileLevels != data.tileLevels
		|| parameters.grid != data.grid;
	
	needUpdate = needCalBounds
		|| parameters.terrainLocalToWorld != localToWorld
		|| parameters.widthPerGrid != data.widthPerGrid
		|| parameters.widthPerTriangle != data.widthPerTriangle
		|| parameters.distanceForFirstTileLevel != data.distanceForFirstTileLevel;

	if (heightMap == NULL) {
		needCalBounds = needUpdate = false;
	}

	this->heightMap = heightMap;
	parameters.terrainLocalToWorld = localToWorld;
	parameters.terrainWorldToLocal = localToWorld.inverse();
	parameters.widthPerGrid = data.widthPerGrid;
	parameters.widthPerTile = data.widthPerTriangle * data.trianglesPerTile;
	parameters.widthPerTriangle = data.widthPerTriangle;
	parameters.height = data.height;
	parameters.tilesPerGrid = data.tilesPerGrid;
	parameters.trianglesPerTile = data.trianglesPerTile;
	parameters.tileLevels = data.tileLevels;
	parameters.grid = data.grid;
	parameters.allGrids = data.grid.x() * data.grid.y();
	parameters.tilesFromAllLevelPerGrid = getLevelOffset(getLevels());
	parameters.distanceForFirstTileLevel = data.distanceForFirstTileLevel;
	
}

void TerrainQuadTree::create()
{
	loadDefaultResource();
	calTileBoundsProgram->init();
	calTileLodBoundsProgram->init();
	selectTilesProgram->init();
}

void TerrainQuadTree::update()
{
	tileBounds.setAutoGenMip(true);
	tileBounds.resize(parameters.tilesPerGrid, parameters.tilesPerGrid, parameters.allGrids);
	parameterBuffer.uploadData(1, &parameters);
}

uint32_t TerrainQuadTree::getLevelOffset(uint32_t level) const
{
	uint32_t levelOffset = 0;
	for (int l = 0; l < level; level++) {
		const uint32_t levelTilesPerGrid = parameters.tilesPerGrid >> level;
		levelOffset += levelTilesPerGrid * levelTilesPerGrid;
	}
	return levelOffset;
}

uint32_t TerrainQuadTree::getLevelOffsetWithCounter(uint32_t level) const
{
	return getLevelOffset(level) + level;
}

uint32_t TerrainQuadTree::getLevels() const
{
	return parameters.tileLevels;
}

void TerrainQuadTree::calTileBounds(IRenderContext& context)
{
	static const ShaderPropertyName parametersName = "Parameters";
	static const ShaderPropertyName tileBoundsName = "tileBounds";
	static const ShaderPropertyName inTileBoundsName = "inTileBounds";
	static const ShaderPropertyName outTileBoundsName = "outTileBounds";
	static const ShaderPropertyName heightMapName = "heightMap";

	if (heightMap == NULL) {
		return;
	}

	RENDER_SCOPE(context, TerrainCalTileBounds);

	{
		context.bindShaderProgram(calTileBoundsProgram);
		context.bindBufferBase(parameterBuffer.getVendorGPUBuffer(), parametersName);
		
		Image image;
		image.texture = &tileBounds;
		image.level = 0;
		image.arrayBase = 0;
		image.arrayCount = parameters.allGrids;
		context.bindImage(image, tileBoundsName);
		context.bindTexture(heightMap->getVendorTexture(), heightMapName);
		context.dispatchCompute(parameters.tilesPerGrid, parameters.tilesPerGrid, parameters.allGrids);
		context.unbindBufferBase(tileBoundsName);
	}

	Vector3u localSize = calTileLodBoundsMaterial->getLocalSize();

	for (int level = 1; level < parameters.tileLevels; level++) {
		const uint32_t tileLodSize = parameters.tilesPerGrid >> level;
		
		context.bindShaderProgram(calTileLodBoundsProgram);
		context.bindBufferBase(parameterBuffer.getVendorGPUBuffer(), parametersName);
		
		Image image;
		image.texture = &tileBounds;
		image.level = level;
		image.arrayBase = 0;
		image.arrayCount = parameters.allGrids;

		MipOption mipOption;
		mipOption.detailMip = level;
		mipOption.mipCount = 1;

		context.bindImage(image, outTileBoundsName);
		context.bindTexture(tileBounds.getVendorTexture(), inTileBoundsName, mipOption);
		context.dispatchCompute(
			ceil(tileLodSize / (float)localSize.x()),
			ceil(tileLodSize / (float)localSize.y()),
			ceil(parameters.allGrids / (float)localSize.z()));
		context.unbindBufferBase(outTileBoundsName);
		context.unbindBufferBase(inTileBoundsName);
	}
}

void TerrainQuadTree::selectTile(IRenderContext& context, CameraRenderData& cameraRenderData, TerrainBatchDrawArray& drawArray)
{
	static const ShaderPropertyName parametersName = "Parameters";
	static const ShaderPropertyName tileBoundsName = "tileBounds";
	static const ShaderPropertyName selectedGridsName = "selectedGrids";
	static const ShaderPropertyName selectedTilesName = "selectedTiles";
	static const ShaderPropertyName selectTileArgsName = "selectTileArgs";
	static const ShaderPropertyName drawArgsName = "drawArgs";

	RENDER_SCOPE(context, TerrainSelectTile);

	drawArray.update(parameters);

	{
		Vector3u localSize = selectGridsMaterial->getLocalSize();
		
		context.clearOutputBufferUint(drawArray.selectTileArgs.getVendorGPUBuffer(), Vector4u::Zero());

		context.bindShaderProgram(selectGridsProgram);
		context.bindBufferBase(parameterBuffer.getVendorGPUBuffer(), parametersName);
		cameraRenderData.bindCameraBuffOnly(context);
		context.bindTexture(tileBounds.getVendorTexture(), tileBoundsName);
		context.bindBufferBase(drawArray.selectedGrids.getVendorGPUBuffer(), selectedGridsName, { true });
		context.bindBufferBase(drawArray.selectTileArgs.getVendorGPUBuffer(), selectTileArgsName, { true });
		context.dispatchCompute(ceil(parameters.allGrids / (float)localSize.x()), 1, 1);

		context.unbindBufferBase(selectedGridsName);
		context.unbindBufferBase(selectTileArgsName);
	}

	{
		context.clearOutputBufferUint(drawArray.drawArgs.getVendorGPUBuffer(), Vector4u::Zero());
		
		context.bindShaderProgram(selectTilesProgram);
		context.bindBufferBase(parameterBuffer.getVendorGPUBuffer(), parametersName);
		cameraRenderData.bindCameraBuffOnly(context);
		context.bindTexture(tileBounds.getVendorTexture(), tileBoundsName);
		context.bindBufferBase(drawArray.selectedGrids.getVendorGPUBuffer(), selectedGridsName);
		context.bindBufferBase(drawArray.selectedTiles.getVendorGPUBuffer(), selectedTilesName, { true });
		context.bindBufferBase(drawArray.drawArgs.getVendorGPUBuffer(), drawArgsName, { true });
		context.dispatchComputeIndirect(drawArray.selectTileArgs.getVendorGPUBuffer(), 0);

		context.unbindBufferBase(selectedGridsName);
		context.unbindBufferBase(selectedTilesName);
		context.unbindBufferBase(drawArgsName);
	}
}

void TerrainQuadTree::loadDefaultResource()
{
	if (isInited)
		return;
	
	calTileBoundsMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/Terrain/CalTerrainTileBounds.mat");
	if (calTileBoundsMaterial == NULL)
		throw runtime_error("Not found default shader");
	calTileBoundsProgram = calTileBoundsMaterial->getShader()->getProgram(Shader_Default);
	if (calTileBoundsProgram == NULL)
		throw runtime_error("Not found default shader");
	
	calTileLodBoundsMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/Terrain/CalTerrainTileLodBounds.mat");
	if (calTileLodBoundsMaterial == NULL)
		throw runtime_error("Not found default shader");
	calTileLodBoundsProgram = calTileLodBoundsMaterial->getShader()->getProgram(Shader_Default);
	if (calTileLodBoundsProgram == NULL)
		throw runtime_error("Not found default shader");
	
	selectGridsMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/Terrain/SelectTerrainGrids.mat");
	if (selectGridsMaterial == NULL)
		throw runtime_error("Not found default shader");
	selectGridsProgram = selectGridsMaterial->getShader()->getProgram(Shader_Default);
	if (selectGridsProgram == NULL)
		throw runtime_error("Not found default shader");
	
	selectTilesMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/Terrain/SelectTerrainTiles.mat");
	if (selectTilesMaterial == NULL)
		throw runtime_error("Not found default shader");
	selectTilesProgram = selectTilesMaterial->getShader()->getProgram(Shader_Default);
	if (selectTilesProgram == NULL)
		throw runtime_error("Not found default shader");
	
	isInited = true;
}

void TerrainRenderData::updateTerrainData(const TerrainData& data, const Matrix4f& localToWorld, Texture2D* heightMap)
{
	quadTree.setTerrainData(data, localToWorld, heightMap);
}

void TerrainRenderData::createForViews(const vector<CameraRenderData*>& cameraRenderDatas)
{
	const int numNeedInit = cameraRenderDatas.size();
	drawArrays.resize(numNeedInit);
	for (int index = 0; index < numNeedInit; index++) {
		TerrainBatchDrawArrayPtr& drawArray = drawArrays[index];
		if (drawArray == nullptr) {
			drawArray = std::make_shared<TerrainBatchDrawArray>();
		}
		drawArray->update(quadTree.getParameters());
	}
}

void TerrainRenderData::create()
{
	quadTree.create();
}

void TerrainRenderData::release()
{
}

void TerrainRenderData::upload()
{
	quadTree.update();
}

void TerrainRenderData::computeTerrainDrawCommands(IRenderContext& context, const vector<CameraRenderData*>& cameraRenderDatas)
{
	if (quadTree.doNeedCalBounds()) {
		quadTree.calTileBounds(context);
	}
	assert(drawArrays.size() == cameraRenderDatas.size());
	for (int index = 0; index < cameraRenderDatas.size(); index++) {
		TerrainBatchDrawArrayPtr& drawArray = drawArrays[index];
		CameraRenderData* cameraRenderData = cameraRenderDatas[index];
		quadTree.selectTile(context, *cameraRenderData, *drawArray);
	}
}

void TerrainRenderData::bind(IRenderContext& context)
{
}
