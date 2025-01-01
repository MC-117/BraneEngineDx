#include "TerrainData.h"
#include "../Utility/MathUtility.h"

void TerrainConfig::validate()
{
    widthPerGrid = std::max(1.0f, widthPerGrid);
    height = std::max(1.0f, height);
    triangleLevels = std::max(1, triangleLevels);
    tileLevels = std::max(1, tileLevels);
    grid.x() = std::min(128u, grid.x());
    grid.y() = std::min(128u, grid.y());
    distanceForFirstTileLevel = std::max(1.0f, distanceForFirstTileLevel);
}

namespace Terrain
{
    uint32_t getMaxLevels(uint32_t size)
    {
        return std::ceil(std::log2(size)) + 1;
    }
	
    uint32_t encodeAddress(uint32_t x, uint32_t y)
    {
        return Math::mortonCode2(x) | (Math::mortonCode2(y) << 1);
    }
	
    Vector2u decodeAddress(uint32_t address)
    {
        return { Math::invertMortonCode2(address), Math::invertMortonCode2(address >> 1)};
    }

    TerrainData getTerrainDataFromConfig(const TerrainConfig& config)
    {
        TerrainData data;
        data.widthPerGrid = config.widthPerGrid;
        data.tileLevels = config.tileLevels;
        data.trianglesPerTile = 1 << (config.triangleLevels - 1);
        data.tilesPerGrid = 1 << (config.tileLevels - 1);
        const int trianglesPerGrid = data.tilesPerGrid * data.trianglesPerTile;
        data.widthPerTriangle = config.widthPerGrid / (float)trianglesPerGrid;
        data.grid = config.grid;
        data.distanceForFirstTileLevel = config.distanceForFirstTileLevel;
        return data;
    }
    
    TerrainConfig convertTerrainConfigFromData(const TerrainData& data)
    {
        TerrainConfig config;
        config.widthPerGrid = data.widthPerGrid;
        config.triangleLevels = std::round(std::log2(data.trianglesPerTile)) + 1;
        config.tileLevels = std::round(std::log2(data.tilesPerGrid)) + 1;
        config.height = data.height;
        config.grid = data.grid;
        config.distanceForFirstTileLevel = data.distanceForFirstTileLevel;
        return config;
    }
}