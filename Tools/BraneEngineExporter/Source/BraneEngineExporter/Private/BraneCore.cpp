#include "BraneCore.h"

namespace Brane
{
	Vector3f::Vector3f(float x, float y, float z) : x(x), y(y), z(z)
	{
	}

	Vector3f& Vector3f::operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	void Vector3f::write(ostream& os)
	{
		os << "x: " << x << ", y: " << y << ", z: " << z;
	}

	void Vector4f::write(ostream& os)
	{
		os << "x: " << x << ", y: " << y << ", z: " << z << ", w: " << w;
	}

	void Quaternionf::write(ostream& os)
	{
		os << "x: " << x << ", y: " << y << ", z: " << z << ", w: " << w;
	}
	
	Color Color::operator+(const Color& c) const
	{
		return Color(r + c.r, g + c.g, b + c.b, a + c.a);
	}

	Color& Color::operator+=(const Color& c)
	{
		r += c.r, g += c.g, b += c.b, a += c.a;
		return *this;
	}

	Color Color::operator-(const Color& c) const
	{
		return Color(r - c.r, g - c.g, b - c.b, a - c.a);
	}

	Color& Color::operator-=(const Color& c)
	{
		r -= c.r, g -= c.g, b -= c.b, a -= c.a;
		return *this;
	}

	Color Color::operator*(const Color& c) const
	{
		return Color(r * c.r, g * c.g, b * c.b, a * c.a);
	}

	Color& Color::operator*=(const Color& c)
	{
		r *= c.r, g *= c.g, b *= c.b, a *= c.a;
		return *this;
	}

	Color Color::operator*(float s) const
	{
		return Color(r * s, g * s, b * s, a * s);
	}

	Color& Color::operator*=(float s)
	{
		r *= s, g *= s, b *= s, a *= s;
		return *this;
	}

	Color Color::operator/(float s) const
	{
		return Color(r / s, g / s, b / s, a / s);
	}

	Color& Color::operator/=(float s)
	{
		r /= s, g /= s, b /= s, a /= s;
		return *this;
	}

	Color Color::operator/(const Color& c) const
	{
		return Color(r / c.r, g / c.g, b / c.b, a / c.a);
	}

	Color& Color::operator/=(const Color& c)
	{
		r /= c.r, g /= c.g, b /= c.b, a /= c.a;
		return *this;
	}

	bool Color::operator==(const Color& c) const
	{
		return r == c.r && g == c.g && b == c.b && a == c.a;
	}

	bool Color::operator!=(const Color& c) const
	{
		return r != c.r || g != c.g || b != c.b || a != c.a;
	}

	void Color::write(ostream& os)
	{
		os << "r: " << r << ", g: " << g << ", b: " << b << ", a: " << a;
	}
}
