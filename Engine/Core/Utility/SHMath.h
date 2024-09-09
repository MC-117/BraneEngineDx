#pragma once

#include "../Unit.h"

struct ENGINE_API SHCoeff3
{
	enum { CoeffCount = 9 };

	union
	{
		Vector4f v[3];
		float d[CoeffCount];
	};

	static SHCoeff3 zero;

	SHCoeff3();
	SHCoeff3(const SHCoeff3& sh);

	SHCoeff3& operator=(const SHCoeff3& sh);
};

struct ENGINE_API SHCoeff3RGB
{
	SHCoeff3 r;
	SHCoeff3 g;
	SHCoeff3 b;

	static SHCoeff3RGB zero;

	bool isZero() const;
};