#pragma once

#include "RenderInterface.h"

struct TerrainConfig
{
    float widthPerGrid = 512;
    float height = 512;
    int tileLevels = 3;
    int triangleLevels = 5;
    Vector2u grid = { 8, 8 };
    float distanceForFirstTileLevel = 100;

    void validate();
};

struct TerrainData
{
    float widthPerGrid = 512;
    float widthPerTriangle = 8;
    float height = 512;
    int tilesPerGrid = 4;
    int trianglesPerTile = 16;
    int tileLevels = 3;
    Vector2u grid = { 8, 8 };
    float distanceForFirstTileLevel = 100;
    Vector3f pad;
};

namespace Terrain
{
    uint32_t getMaxLevels(uint32_t size);
    uint32_t encodeAddress(uint32_t x, uint32_t y);
    Vector2u decodeAddress(uint32_t address);
    TerrainData getTerrainDataFromConfig(const TerrainConfig& config);
    TerrainConfig convertTerrainConfigFromData(const TerrainData& data);
}

struct TerrainPatchLodData
{
    union
    {
        struct
        {
            uint16_t lod : 16;
            uint8_t lodOffTop : 4;
            uint8_t lodOffLeft : 4;
            uint8_t lodOffRight : 4;
            uint8_t lodOffBottom : 4;
        };
        uint32_t packedValue;
    };
};

struct TerrainPatchTile
{
    union
    {
        struct
        {
            uint32_t address : 28;
            uint32_t level : 4;
        };
        uint32_t packedValue;
    };
    TerrainPatchTile() : packedValue(0) {}
    TerrainPatchTile(uint32_t level, uint32_t address) : address(address), level(level) {}
    TerrainPatchTile(uint32_t level, uint32_t x, uint32_t y) : address(Terrain::encodeAddress(x, y)), level(level) {}
};
