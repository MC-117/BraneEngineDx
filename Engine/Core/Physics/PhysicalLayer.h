#pragma once
#include "../Config.h"
#include "../Serialization.h"

struct ENGINE_API PhysicalLayer
{
public:
	struct LayerBool
	{
		bool enable = true;

		LayerBool() {}
		LayerBool(const bool& enable) : enable(enable) {}
		operator bool() { return enable; }
	};

	static const unsigned int layerMaxCount = 32;
	static const unsigned int layerNoIgnoreMask = 0;
	static const unsigned int layerAllIgnoreMask = 0xffffffff;
	static LayerBool collisionMatrix[layerMaxCount][layerMaxCount];

	uint32_t layer = 0;
	uint32_t customLayer = 0;
	uint32_t ignoreMask = 0;
	uint32_t customMask = 0;

	PhysicalLayer();
	PhysicalLayer(uint8_t layer, uint32_t ignoreMask = layerNoIgnoreMask);
	PhysicalLayer(const PxFilterData& fd);

	void setLayer(uint8_t layer);
	uint8_t getLayer();

	void setIgnoreLayer(uint8_t layer, bool ignore);
	bool isIgnoreLayer(uint8_t layer);

	operator PxFilterData() const;

	void serialize(SerializationInfo& info);
	void deserialize(const SerializationInfo& info);

	static void setCollisionEnable(uint8_t layer0, uint8_t layer1, bool enable);
	static bool getCollisionEnable(uint8_t layer0, uint8_t layer1);

	static PxFilterFlags SimulationFilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);
protected:

	static bool checkLayer(uint8_t layer);
};

