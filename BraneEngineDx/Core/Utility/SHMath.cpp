#include "SHMath.h"
#include "../Unit.h"

SHCoeff3 SHCoeff3::zero = SHCoeff3();

SHCoeff3RGB SHCoeff3RGB::zero = { SHCoeff3::zero, SHCoeff3::zero, SHCoeff3::zero };

SHCoeff3::SHCoeff3()
{
	v[0] = Vector4f::Zero();
	v[1] = Vector4f::Zero();
	v[2] = Vector4f::Zero();
}

SHCoeff3::SHCoeff3(const SHCoeff3& sh)
{
	memcpy(this, &sh, sizeof(SHCoeff3));
}

bool SHCoeff3RGB::isZero() const
{
	return memcmp(this, &zero, sizeof(SHCoeff3RGB)) == 0;
}
