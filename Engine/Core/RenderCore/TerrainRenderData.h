#pragma once

#include "RenderInterface.h"
#include "CameraData.h"
#include "TerrainData.h"
#include "../GPUBuffer.h"
#include "../Texture2D.h"
#include "../Utility/Boundings.h"

struct TerrainCalTileBoundParameters
{
    Matrix4f terrainLocalToWorld;
    Matrix4f terrainWorldToLocal;
    float widthPerGrid = 0;
    float widthPerTile = 0;
    float widthPerTriangle = 0;
    float height = 0;
    int tilesFromAllLevelPerGrid = 0;
    int tilesPerGrid = 0;
    int trianglesPerTile = 0;
    int tileLevels = 0;
    float distanceForFirstTileLevel = 0;
    int allGrids = 0;
    Vector2u grid;
};

struct TerrainBatchDrawArray : public IBatchDrawCommandArray
{
    GPUBuffer selectedGrids;
    GPUBuffer selectedTiles;
    GPUBuffer selectTileArgs;
    GPUBuffer drawArgs;

    bool needCalBounds;
    bool needUpdate;

    TerrainBatchDrawArray();

    virtual void update(const TerrainCalTileBoundParameters& parameters);
    
    virtual void bindInstanceBuffer(IRenderContext& context);
    virtual IGPUBuffer* getInstanceBuffer() { return selectedTiles.getVendorGPUBuffer(); }
    virtual IGPUBuffer* getCommandBuffer() { return drawArgs.getVendorGPUBuffer(); }
    virtual unsigned int getInstanceCount() const;
    virtual unsigned int getCommandCount() const;
};

using TerrainBatchDrawArrayPtr = std::shared_ptr<TerrainBatchDrawArray>;

struct TerrainQuadTree
{
public:
    TerrainQuadTree();

    bool doNeedCalBounds() { return needCalBounds; }

    void setTerrainData(const TerrainData& data, const Matrix4f& localToWorld, Texture2D* heightMap);

    const TerrainCalTileBoundParameters& getParameters() const { return parameters; }

    void create();
    void update();
    uint32_t getLevelOffset(uint32_t level) const;
    uint32_t getLevelOffsetWithCounter(uint32_t level) const;
    uint32_t getLevels() const;

    void calTileBounds(IRenderContext& context);

    void selectTile(IRenderContext& context, CameraRenderData& cameraRenderData, TerrainBatchDrawArray& drawArray);
protected:
    TerrainCalTileBoundParameters parameters;

    bool needCalBounds = true;
    bool needUpdate = true;

    Texture2D* heightMap = NULL;
    Texture2D tileBounds;
    GPUBuffer parameterBuffer;

    static Material* calTileBoundsMaterial;
    static ShaderProgram* calTileBoundsProgram;
    static Material* calTileLodBoundsMaterial;
    static ShaderProgram* calTileLodBoundsProgram;
    static Material* selectGridsMaterial;
    static ShaderProgram* selectGridsProgram;
    static Material* selectTilesMaterial;
    static ShaderProgram* selectTilesProgram;
    static bool isInited;

    static void loadDefaultResource();
};

struct TerrainRenderData : public IRenderData
{
    TerrainData terrainData;
    TerrainQuadTree quadTree;
    vector<TerrainBatchDrawArrayPtr> drawArrays;

    void updateTerrainData(const TerrainData& data, const Matrix4f& localToWorld, Texture2D* heightMap);

    void createForViews(const vector<CameraRenderData*>& cameraRenderDatas);

    virtual void create();
    virtual void release();
    virtual void upload();
    void computeTerrainDrawCommands(IRenderContext& context, const vector<CameraRenderData*>& cameraRenderDatas);
    virtual void bind(IRenderContext& context);
};
