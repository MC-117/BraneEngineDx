#pragma once

#include "../Unit.h"

enum ProbeType
{
	ProbeType_Light,
	ProbeType_Ref,
	ProbeType_Env,
	ProbeType_Count,
	ProbeType_Range,
};

struct LocalLightData
{
	Vector3f position;
	float radius;
	Vector3f color;
	float intensity = 0;
	int vsmID;
};

struct ReflectionProbeData
{
	Vector3f position;
	float radius = 0;
	Color tintColor;
	float falloff = 5;
	float cutoff = 0;
	int cubeMapIndex = -1;
};

struct EnvLightData
{
	Vector3f position;
	float radius = 0;
	Vector3f tintColor;
	int reverseIndex = 0;
	float falloff = 0;
	float cutoff = 0;
	int shDataOffset = -1;
};

struct ProbeByteData
{
	union
	{
		LocalLightData localLightData;
		ReflectionProbeData reflectionProbeData;
		EnvLightData envLightData;
	};
	ProbeType probeType;

	ProbeByteData() { memset(this, 0, sizeof(ProbeByteData)); }
	ProbeByteData(const ProbeByteData& other) { memcpy(this, &other, sizeof(ProbeByteData)); }
};

static constexpr int ProbeByteSize = sizeof(ProbeByteData);