#pragma once
#include <iostream>

namespace Brane
{
	using namespace std;

	class IWritable
	{
	public:
		IWritable() = default;
		virtual ~IWritable() = default;
		virtual const char* getClass() const = 0;
		virtual void write(ostream& os) = 0;

		friend ostream& operator<<(ostream& os, IWritable& obj)
		{
			os << obj.getClass() << " { ";
			obj.write(os);
			os << " }";
			return os;
		}
	};

#define BESerialize(Type) virtual const char* getClass() const { return #Type; }

	class Vector3f : public IWritable
	{
	public:
		BESerialize(SVector3f);
		
		float x = 0, y = 0, z = 0;

		Vector3f(float x = 0, float y = 0, float z = 0);
		virtual ~Vector3f() = default;

		Vector3f& operator*=(float scale);

		virtual void write(ostream& os);
	};

	class Vector4f : public IWritable
	{
	public:
		BESerialize(SVector4f);
		
		float x = 0, y = 0, z = 0, w = 0;

		virtual ~Vector4f() = default;

		virtual void write(ostream& os);
	};

	class Quaternionf : public IWritable
	{
	public:
		BESerialize(SQuaternionf);
		
		float x = 0, y = 0, z = 0, w = 1;

		virtual ~Quaternionf() = default;

		virtual void write(ostream& os);
	};

	class Color : public IWritable
	{
	public:
		BESerialize(SColor);
		
		float r = 0, g = 0, b = 0, a = 0;

		Color() {}
		Color(int r, int g, int b, int a = 255) : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a / 255.0f) {}
		Color(float r, float g, float b, float a = 1) : r(r), g(g), b(b), a(a) {}
		virtual ~Color() = default;

		Color operator+(const Color& c) const;
		Color& operator+=(const Color& c);
		Color operator-(const Color& c) const;
		Color& operator-=(const Color& c);
		Color operator*(const Color& c) const;
		Color& operator*=(const Color& c);
		Color operator*(float s) const;
		Color& operator*=(float s);
		Color operator/(const Color& c) const;
		Color& operator/=(const Color& c);
		Color operator/(float s) const;
		Color& operator/=(float s);
		bool operator==(const Color& c) const;
		bool operator!=(const Color& c) const;

		virtual void write(ostream& os);
	};
}
