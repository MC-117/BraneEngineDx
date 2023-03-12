#pragma once
#ifndef _MATHLIBRARY_H_

#include <DirectXMath.h>

/*
* This engine task is based on my own game engine (BraneEngine).
* BraneEngine use OpenGL as vendor and Eigen as math library, so
* that I capsulate the DirectXMath library with Eigen style to
* fit the reused code.
* Note: the new MathLibrary provide linear operation in column
* major, while the actual implementation with DirectXMath is in
* row major.
*/

struct Vector2f;
struct Vector3f;
struct Vector4f;
struct Matrix3f;
struct Matrix4f;
struct Quaternionf;

struct Block
{
	float* d = nullptr;
	unsigned int rowCount = 0, colCount = 0;
	unsigned int rowStart = 0, rowNum = 0;
	unsigned int colStart = 0, colNum = 0;

	Block(float* d, unsigned int rowCount, unsigned int colCount,
		unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum);

	void validate() const;

	Block row(unsigned int index);
	Block col(unsigned int index);

	unsigned int rows() const;
	unsigned int cols() const;

	Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum);
	const Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum) const;

	float& operator()(unsigned int r, unsigned int c);
	float operator()(unsigned int r, unsigned int c) const;

	Block& operator=(const Block& b);

	bool operator==(const Block& b) const;
	bool operator!=(const Block& b) const;
};

struct Vector2f : protected DirectX::XMFLOAT2
{
	Vector2f(float x = 0, float y = 0);
	Vector2f(const Vector2f& v);
	Vector2f(const Block& b);
	Vector2f(const DirectX::XMFLOAT2& xmf2);

	float& x();
	float x() const;

	float& y();
	float y() const;

	static Vector2f Identity();
	static Vector2f Zero();
	static Vector2f Ones();
	static Vector2f UnitX();
	static Vector2f UnitY();

	void setZero();

	float* data() const;

	float dot(const Vector2f& v) const;
	Vector2f cross(const Vector2f& v) const;
	float squaredNorm() const;
	float norm() const;
	Vector2f normalized() const;
	Vector2f& normalize();

	Vector2f cwiseProduct(const Vector2f& v) const;

	Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum);
	const Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum) const;

	Vector2f& operator=(const Vector2f& v);

	Vector2f operator-();

	Vector2f operator+(const Vector2f& v) const;
	Vector2f& operator+=(const Vector2f& v);

	Vector2f operator-(const Vector2f& v) const;
	Vector2f& operator-=(const Vector2f& v);

	Vector2f operator*(float s) const;
	Vector2f& operator*=(float s);

	Vector2f operator/(float s) const;
	Vector2f& operator/=(float s);

	bool operator==(const Vector2f& v) const;
	bool operator!=(const Vector2f& v) const;

	float& operator[](unsigned int index);
	float operator[](unsigned int index) const;

	operator Block();
	operator Block() const;
};

template<class T>
struct Vector2T
{
protected:
	T _x = 0, _y = 0;
public:
	Vector2T(T x = 0, T y = 0)
		: _x(x), _y(y)
	{
	}
	Vector2T(const Vector2T& v)
	{
		_x = v._x;
		_y = v._y;
	}

	T& x()
	{
		return _x;
	}
	T x() const
	{
		return _x;
	}

	T& y()
	{
		return _y;
	}
	T y() const
	{
		return _y;
	}

	static Vector2T Identity()
	{
		return Vector2T(1, 0);
	}
	static Vector2T Zero()
	{
		return Vector2T();
	}
	static Vector2T Ones()
	{
		return Vector2T(1, 1);
	}
	static Vector2T UnitX()
	{
		return Vector2T(1, 0);
	}
	static Vector2T UnitY()
	{
		return Vector2T(0, 1);
	}

	void setZero()
	{
		_x = _y = 0;
	}

	T* data() const
	{
		return (unsigned int*)this;
	}

	float dot(const Vector2T& v) const
	{
		return Vector2f(_x, _y).dot(Vector2f(v._x, v._y));
	}
	Vector2f cross(const Vector2T& v) const
	{
		return Vector2f(_x, _y).cross(Vector2f(v._x, v._y));
	}
	float squaredNorm() const
	{
		return Vector2f(_x, _y).squaredNorm();
	}
	float norm() const
	{
		return Vector2f(_x, _y).norm();
	}
	Vector2f normalized() const
	{
		return Vector2f(_x, _y).normalized();
	}

	Vector2T cwiseProduct(const Vector2T& v) const
	{
		return Vector2T(_x * v._x, _y * v._y);
	}

	Vector2T& operator=(const Vector2T& v)
	{
		_x = v._x;
		_y = v._y;
		return *this;
	}

	Vector2T operator-()
	{
		return Vector2f(-_x, -_y);
	}

	Vector2T operator+(const Vector2T& v) const
	{
		return Vector2T(_x + v._x, _y + v._y);
	}
	Vector2T& operator+=(const Vector2T& v)
	{
		_x += v._x;
		_y += v._y;
		return *this;
	}

	Vector2T operator-(const Vector2T& v) const
	{
		return Vector2T(_x - v._x, _y - v._y);
	}
	Vector2T& operator-=(const Vector2T& v)
	{
		_x -= v._x;
		_y -= v._y;
		return *this;
	}

	Vector2T operator*(T s) const
	{
		return Vector2T(_x * s, _y * s);
	}
	Vector2T& operator*=(T s)
	{
		_x *= s;
		_y *= s;
		return *this;
	}

	Vector2T operator/(T s) const
	{
		return Vector2T(_x / s, _y / s);
	}
	Vector2T& operator/=(T s)
	{
		_x /= s;
		_y /= s;
		return *this;
	}

	bool operator==(const Vector2T& v) const
	{
		return _x == v._x && _y == v._y;
	}
	bool operator!=(const Vector2T& v) const
	{
		return _x != v._x || _y != v._y;
	}

	T& operator[](unsigned int index)
	{
		if (index > 1)
			throw overflow_error("Vector2u only has 2 elements");
		return ((unsigned int*)this)[index];
	}
	T operator[](unsigned int index) const
	{
		if (index > 1)
			throw overflow_error("Vector2u only has 2 elements");
		return ((unsigned int*)this)[index];
	}
};

typedef Vector2T<int> Vector2i;
typedef Vector2T<long long> Vector2l;
typedef Vector2T<unsigned int> Vector2u;
typedef Vector2T<unsigned long long> Vector2ul;

struct Vector3f : protected DirectX::XMFLOAT3
{
	Vector3f(float x = 0, float y = 0, float z = 0);
	Vector3f(const Vector3f& v);
	Vector3f(const Block& b);
	Vector3f(const DirectX::XMFLOAT3& xmf3);

	float& x();
	float x() const;

	float& y();
	float y() const;

	float& z();
	float z() const;

	static Vector3f Identity();
	static Vector3f Zero();
	static Vector3f Ones();
	static Vector3f UnitX();
	static Vector3f UnitY();
	static Vector3f UnitZ();

	void setZero();

	float* data() const;

	float dot(const Vector3f& v) const;
	Vector3f cross(const Vector3f& v) const;
	float squaredNorm() const;
	float norm() const;
	Vector3f normalized() const;
	Vector3f& normalize();

	Vector3f cwiseProduct(const Vector3f& v) const;

	Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum);
	const Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum) const;

	Vector3f& operator=(const Vector3f& v);

	Vector3f operator-();

	Vector3f operator+(const Vector3f& v) const;
	Vector3f& operator+=(const Vector3f& v);

	Vector3f operator-(const Vector3f& v) const;
	Vector3f& operator-=(const Vector3f& v);

	Vector3f operator*(float s) const;
	Vector3f& operator*=(float s);

	Vector3f operator/(float s) const;
	Vector3f& operator/=(float s);

	bool operator==(const Vector3f& v) const;
	bool operator!=(const Vector3f& v) const;

	float& operator[](unsigned int index);
	float operator[](unsigned int index) const;

	operator Block();
	operator Block() const;
};

template<class T>
struct Vector3T
{
protected:
	T _x = 0, _y = 0, _z = 0;
public:
	Vector3T(T x = 0, T y = 0, T z = 0)
		: _x(x), _y(y), _z(z)
	{
	}
	Vector3T(const Vector3T& v)
	{
		_x = v._x;
		_y = v._y;
		_z = v._z;
	}

	T& x()
	{
		return _x;
	}
	T x() const
	{
		return _x;
	}

	T& y()
	{
		return _y;
	}
	T y() const
	{
		return _y;
	}

	T& z()
	{
		return _z;
	}
	T z() const
	{
		return _z;
	}

	static Vector3T Identity()
	{
		return Vector3T(1, 0, 0);
	}
	static Vector3T Zero()
	{
		return Vector3T();
	}
	static Vector3T Ones()
	{
		return Vector3T(1, 1, 1);
	}
	static Vector3T UnitX()
	{
		return Vector3T(1, 0, 0);
	}
	static Vector3T UnitY()
	{
		return Vector3T(0, 1, 0);
	}
	static Vector3T UnitZ()
	{
		return Vector3T(0, 0, 1);
	}

	void setZero()
	{
		_x = _y = _z = 0;
	}

	T* data() const
	{
		return (unsigned int*)this;
	}

	float dot(const Vector3T& v) const
	{
		return Vector3f(_x, _y, _z).dot(Vector3f(v._x, v._y, v._z));
	}
	Vector3f cross(const Vector3T& v) const
	{
		return Vector3f(_x, _y, _z).cross(Vector3f(v._x, v._y, v._z));
	}
	float squaredNorm() const
	{
		return Vector3f(_x, _y, _z).squaredNorm();
	}
	float norm() const
	{
		return Vector3f(_x, _y, _z).norm();
	}
	Vector3f normalized() const
	{
		return Vector3f(_x, _y, _z).normalized();
	}

	Vector3T cwiseProduct(const Vector3T& v) const
	{
		return Vector3T(_x * v._x, _y * v._y, _z * v._z);
	}

	Vector3T& operator=(const Vector3T& v)
	{
		_x = v._x;
		_y = v._y;
		_z = v._z;
		return *this;
	}

	Vector3T operator-()
	{
		return Vector3T(-_x, -_y, -_z);
	}

	Vector3T operator+(const Vector3T& v) const
	{
		return Vector3T(_x + v._x, _y + v._y, _z + v._z);
	}
	Vector3T& operator+=(const Vector3T& v)
	{
		_x += v._x;
		_y += v._y;
		_z += v._z;
		return *this;
	}

	Vector3T operator-(const Vector3T& v) const
	{
		return Vector3T(_x - v._x, _y - v._y, _z - v._z);
	}
	Vector3T& operator-=(const Vector3T& v)
	{
		_x -= v._x;
		_y -= v._y;
		_z -= v._z;
		return *this;
	}

	Vector3T operator*(T s) const
	{
		return Vector3T(_x * s, _y * s, _z * s);
	}
	Vector3T& operator*=(T s)
	{
		_x *= s;
		_y *= s;
		_z *= s;
		return *this;
	}

	Vector3T operator/(T s) const
	{
		return Vector3T(_x / s, _y / s, _z / s);
	}
	Vector3T& operator/=(T s)
	{
		_x /= s;
		_y /= s;
		_z /= s;
		return *this;
	}

	bool operator==(const Vector3T& v) const
	{
		return _x == v._x && _y == v._y && _z == v._z;
	}
	bool operator!=(const Vector3T& v) const
	{
		return _x != v._x || _y != v._y || _z != v._z;
	}

	T& operator[](unsigned int index)
	{
		if (index > 2)
			throw overflow_error("Vector3u only has 3 elements");
		return ((unsigned int*)this)[index];
	}
	T operator[](unsigned int index) const
	{
		if (index > 2)
			throw overflow_error("Vector3u only has 3 elements");
		return ((unsigned int*)this)[index];
	}
};

typedef Vector3T<int> Vector3i;
typedef Vector3T<long long> Vector3l;
typedef Vector3T<unsigned int> Vector3u;
typedef Vector3T<unsigned long long> Vector3ul;

struct Vector4f : protected DirectX::XMFLOAT4
{
	Vector4f(float x = 0, float y = 0, float z = 0, float w = 0);
	Vector4f(const Vector3f& v, float w);
	Vector4f(const Vector4f& v);
	Vector4f(const Block& b);
	Vector4f(const DirectX::XMFLOAT4& xmf4);

	float& x();
	float x() const;

	float& y();
	float y() const;

	float& z();
	float z() const;

	float& w();
	float w() const;

	static Vector4f Identity();
	static Vector4f Zero();
	static Vector4f Ones();
	static Vector4f UnitX();
	static Vector4f UnitY();
	static Vector4f UnitZ();
	static Vector4f UnitW();
	static Vector4f Plane(const Vector3f& n, const Vector3f& p);
	static Vector4f Plane(const Vector3f& p0, const Vector3f& p1, const Vector3f& p2);

	void setZero();

	float* data() const;

	float dot(const Vector4f& v) const;
	float squaredNorm() const;
	float norm() const;
	Vector4f normalized() const;
	Vector4f& normalize();
	
	Vector4f cwiseProduct(const Vector4f& v) const;
	Vector4f& setPlane(const Vector3f& n, const Vector3f& p);
	Vector4f& setPlane(const Vector3f& p0, const Vector3f& p1, const Vector3f& p2);

	Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum);
	const Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum) const;

	Vector4f& operator=(const Vector4f& v);

	Vector4f operator-();

	Vector4f operator+(const Vector4f& v) const;
	Vector4f& operator+=(const Vector4f& v);

	Vector4f operator-(const Vector4f& v) const;
	Vector4f& operator-=(const Vector4f& v);

	Vector4f operator*(float s) const;
	Vector4f& operator*=(float s);

	Vector4f operator/(float s) const;
	Vector4f& operator/=(float s);

	bool operator==(const Vector4f& v) const;
	bool operator!=(const Vector4f& v) const;

	float& operator[](unsigned int index);
	float operator[](unsigned int index) const;

	operator Block();
	operator Block() const;
};

template<class T>
struct Vector4T
{
protected:
	T _x = 0, _y = 0, _z = 0, _w = 0;
public:
	Vector4T(T x = 0, T y = 0, T z = 0, T w = 0)
		: _x(x), _y(y), _z(z), _w(w)
	{
	}
	Vector4T(const Vector4T& v)
	{
		_x = v._x;
		_y = v._y;
		_z = v._z;
		_w = v._w;
	}

	T& x()
	{
		return _x;
	}
	T x() const
	{
		return _x;
	}

	T& y()
	{
		return _y;
	}
	T y() const
	{
		return _y;
	}

	T& z()
	{
		return _z;
	}
	T z() const
	{
		return _z;
	}

	T& w()
	{
		return _w;
	}
	T w() const
	{
		return _w;
	}

	static Vector4T Identity()
	{
		return Vector4T(1, 0, 0, 0);
	}
	static Vector4T Zero()
	{
		return Vector4T();
	}
	static Vector4T Ones()
	{
		return Vector4T(1, 1, 1, 1);
	}
	static Vector4T UnitX()
	{
		return Vector4T(1, 0, 0, 0);
	}
	static Vector4T UnitY()
	{
		return Vector4T(0, 1, 0, 0);
	}
	static Vector4T UnitZ()
	{
		return Vector4T(0, 0, 1, 0);
	}
	static Vector4T UnitW()
	{
		return Vector4T(0, 0, 0, 1);
	}

	void setZero()
	{
		_x = _y = _z = _w = 0;
	}

	T* data() const
	{
		return (unsigned int*)this;
	}

	float dot(const Vector4T& v) const
	{
		return Vector4f(_x, _y, _z, _w).dot(Vector4f(v._x, v._y, v._z, _w));
	}
	float squaredNorm() const
	{
		return Vector4f(_x, _y, _z, _w).squaredNorm();
	}
	float norm() const
	{
		return Vector4f(_x, _y, _z, _w).norm();
	}
	Vector4f normalized() const
	{
		return Vector4f(_x, _y, _z, _w).normalized();
	}

	Vector4T cwiseProduct(const Vector4T& v) const
	{
		return Vector4T(_x * v._x, _y * v._y, _z * v._z, _w * v._w);
	}

	Vector4T& operator=(const Vector4T& v)
	{
		_x = v._x;
		_y = v._y;
		_z = v._z;
		_w = v._w;
		return *this;
	}

	Vector4T operator-()
	{
		return Vector4T(-_x, -_y, -_z, -_w);
	}

	Vector4T operator+(const Vector4T& v) const
	{
		return Vector4T(_x + v._x, _y + v._y, _z + v._z, _w + v._w);
	}
	Vector4T& operator+=(const Vector4T& v)
	{
		_x += v._x;
		_y += v._y;
		_z += v._z;
		_w += v._w;
		return *this;
	}

	Vector4T operator-(const Vector4T& v) const
	{
		return Vector4T(_x - v._x, _y - v._y, _z - v._z, _w - v._w);
	}
	Vector4T& operator-=(const Vector4T& v)
	{
		_x -= v._x;
		_y -= v._y;
		_z -= v._z;
		_w -= v._w;
		return *this;
	}

	Vector4T operator*(T s) const
	{
		return Vector4T(_x * s, _y * s, _z * s, _w * s);
	}
	Vector4T& operator*=(T s)
	{
		_x *= s;
		_y *= s;
		_z *= s;
		_w *= s;
		return *this;
	}

	Vector4T operator/(T s) const
	{
		return Vector4T(_x / s, _y / s, _z / s, _w / s);
	}
	Vector4T& operator/=(T s)
	{
		_x /= s;
		_y /= s;
		_z /= s;
		_w /= s;
		return *this;
	}

	bool operator==(const Vector4T& v) const
	{
		return _x == v._x && _y == v._y && _z == v._z && _w == v._w;
	}
	bool operator!=(const Vector4T& v) const
	{
		return _x != v._x || _y != v._y || _z != v._z || _w != v._w;
	}

	T& operator[](unsigned int index)
	{
		if (index > 3)
			throw overflow_error("Vector4u only has 4 elements");
		return ((T*)this)[index];
	}
	T operator[](unsigned int index) const
	{
		if (index > 3)
			throw overflow_error("Vector4u only has 4 elements");
		return ((T*)this)[index];
	}
};

typedef Vector4T<int> Vector4i;
typedef Vector4T<long long> Vector4l;
typedef Vector4T<unsigned int> Vector4u;
typedef Vector4T<unsigned long long> Vector4ul;

struct Matrix3f : protected DirectX::XMFLOAT3X3
{
	Matrix3f();
	Matrix3f(const Matrix3f& m);
	Matrix3f(const float* pArray);
	Matrix3f(const Block& b);
	Matrix3f(const DirectX::XMFLOAT3X3& xmf3x3);
	Matrix3f(const DirectX::XMMATRIX& xmm);

	static Matrix3f Identity();
	static Matrix3f Zero();

	float* data() const;

	Matrix3f inverse() const;
	Matrix3f transpose() const;
	Matrix3f& transposeInPlace();
	Vector3f eulerAngles() const;

	Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum);
	const Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum) const;

	float& operator()(unsigned int row, unsigned int col);
	float operator()(unsigned int row, unsigned int col) const;

	Matrix3f& operator=(const Matrix3f& m);

	Matrix3f operator*(float s) const;
	Matrix3f& operator*=(float s);

	Matrix3f operator*(const Quaternionf& q) const;
	Matrix3f& operator*(const Quaternionf& q);

	Vector3f operator*(const Vector3f& v) const;
	Matrix3f operator*(const Matrix3f& m) const;
	Matrix3f& operator*=(const Matrix3f& m);

	operator Block();
	operator Block() const;
};

struct Matrix4f : protected DirectX::XMFLOAT4X4
{
	Matrix4f();
	Matrix4f(const Matrix4f& m);
	Matrix4f(const float* pArray);
	Matrix4f(const Block& b);
	Matrix4f(const DirectX::XMFLOAT4X4& xmf4x4);
	Matrix4f(const DirectX::XMMATRIX& xmm);

	static Matrix4f Identity();
	static Matrix4f Zero();

	float* data() const;

	Matrix4f inverse() const;
	Matrix4f transpose() const;
	Matrix4f& transposeInPlace();
	bool decompose(Vector3f& pos, Quaternionf& rot, Vector3f& sca) const;

	Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum);
	const Block block(unsigned int rowStart, unsigned int colStart,
		unsigned int rowNum, unsigned int colNum) const;

	float& operator()(unsigned int row, unsigned int col);
	float operator()(unsigned int row, unsigned int col) const;

	Matrix4f& operator=(const Matrix4f& m);

	Matrix4f operator*(float s) const;
	Matrix4f& operator*=(float s);

	Vector4f operator*(const Vector4f& v) const;
	Matrix4f operator*(const Matrix4f& m) const;
	Matrix4f& operator*=(const Matrix4f& m);

	bool operator==(const Matrix4f& m) const;
	bool operator!=(const Matrix4f& m) const;

	operator Block();
	operator Block() const;
};

struct Quaternionf : public Vector4f
{
	Quaternionf(float w = 0, float x = 0, float y = 0, float z = 0);
	Quaternionf(const Quaternionf& q);
	Quaternionf(const DirectX::XMFLOAT4& xmf4);
	Quaternionf(const Matrix3f& m);

	static Quaternionf Identity();
	static Quaternionf FromEularAngles(const Vector3f& rollPicthYaw);
	static Quaternionf FromTwoVectors(const Vector3f& a, const Vector3f& b);
	static Quaternionf FromAngleAxis(float angle, const Vector3f& axis);

	Quaternionf normalized() const;
	Quaternionf& normalize();

	Quaternionf& setIdentity();
	Quaternionf& setFromEularAngles(const Vector3f& rollPicthYaw);
	Quaternionf& setFromTwoVectors(const Vector3f& a, const Vector3f& b);
	Quaternionf& setFromAngleAxis(float angle, const Vector3f& axis);
	Quaternionf inverse() const;
	Quaternionf slerp(float t, const Quaternionf& q) const;
	Matrix3f toRotationMatrix() const;

	Quaternionf& operator=(const Quaternionf& q);
	Quaternionf& operator=(const Matrix3f& m);

	Vector3f operator*(const Vector3f& v) const;

	Quaternionf operator*(const Quaternionf& q) const;
	Quaternionf& operator*=(const Quaternionf& q);

	Quaternionf operator*(float s) const;
	Quaternionf& operator*=(const float& s);
};

#endif // !_MATHLIBRARY_H_
