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
struct Vector2u;
struct Vector3f;
struct Vector3u;
struct Vector4f;
struct Vector4u;
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

struct Vector2u
{
protected:
	unsigned int _x = 0, _y = 0;
public:
	Vector2u(unsigned int x = 0, unsigned int y = 0);
	Vector2u(const Vector2u& v);

	unsigned int& x();
	unsigned int x() const;

	unsigned int& y();
	unsigned int y() const;

	static Vector2u Identity();
	static Vector2u Zero();
	static Vector2u Ones();
	static Vector2u UnitX();
	static Vector2u UnitY();

	void setZero();

	unsigned int* data() const;

	float dot(const Vector2u& v) const;
	Vector2f cross(const Vector2u& v) const;
	float squaredNorm() const;
	float norm() const;
	Vector2f normalized() const;

	Vector2u cwiseProduct(const Vector2u& v) const;

	Vector2u& operator=(const Vector2u& v);

	Vector2f operator-();

	Vector2u operator+(const Vector2u& v) const;
	Vector2u& operator+=(const Vector2u& v);

	Vector2u operator-(const Vector2u& v) const;
	Vector2u& operator-=(const Vector2u& v);

	Vector2u operator*(unsigned int s) const;
	Vector2u& operator*=(unsigned int s);

	Vector2u operator/(unsigned int s) const;
	Vector2u& operator/=(unsigned int s);

	bool operator==(const Vector2u& v) const;
	bool operator!=(const Vector2u& v) const;

	unsigned int& operator[](unsigned int index);
	unsigned int operator[](unsigned int index) const;
};

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

struct Vector3u
{
protected:
	unsigned int _x = 0, _y = 0, _z = 0;
public:
	Vector3u(unsigned int x = 0, unsigned int y = 0, unsigned int z = 0);
	Vector3u(const Vector3u& v);

	unsigned int& x();
	unsigned int x() const;

	unsigned int& y();
	unsigned int y() const;

	unsigned int& z();
	unsigned int z() const;

	static Vector3u Identity();
	static Vector3u Zero();
	static Vector3u Ones();
	static Vector3u UnitX();
	static Vector3u UnitY();
	static Vector3u UnitZ();

	void setZero();

	unsigned int* data() const;

	float dot(const Vector3u& v) const;
	Vector3f cross(const Vector3u& v) const;
	float squaredNorm() const;
	float norm() const;
	Vector3f normalized() const;

	Vector3u cwiseProduct(const Vector3u& v) const;

	Vector3u& operator=(const Vector3u& v);

	Vector3f operator-();

	Vector3u operator+(const Vector3u& v) const;
	Vector3u& operator+=(const Vector3u& v);

	Vector3u operator-(const Vector3u& v) const;
	Vector3u& operator-=(const Vector3u& v);

	Vector3u operator*(unsigned int s) const;
	Vector3u& operator*=(unsigned int s);

	Vector3u operator/(unsigned int s) const;
	Vector3u& operator/=(unsigned int s);

	bool operator==(const Vector3u& v) const;
	bool operator!=(const Vector3u& v) const;

	unsigned int& operator[](unsigned int index);
	unsigned int operator[](unsigned int index) const;
};

struct Vector4f : protected DirectX::XMFLOAT4
{
	Vector4f(float x = 0, float y = 0, float z = 0, float w = 0);
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

struct Vector4u
{
protected:
	unsigned int _x = 0, _y = 0, _z = 0, _w = 0;
public:
	Vector4u(unsigned int x = 0, unsigned int y = 0, unsigned int z = 0, unsigned int w = 0);
	Vector4u(const Vector4u& v);

	unsigned int& x();
	unsigned int x() const;

	unsigned int& y();
	unsigned int y() const;

	unsigned int& z();
	unsigned int z() const;

	unsigned int& w();
	unsigned int w() const;

	static Vector4u Identity();
	static Vector4u Zero();
	static Vector4u Ones();
	static Vector4u UnitX();
	static Vector4u UnitY();
	static Vector4u UnitZ();
	static Vector4u UnitW();

	void setZero();

	unsigned int* data() const;

	float dot(const Vector4u& v) const;
	float squaredNorm() const;
	float norm() const;
	Vector4f normalized() const;

	Vector4u cwiseProduct(const Vector4u& v) const;

	Vector4u& operator=(const Vector4u& v);

	Vector4f operator-();

	Vector4u operator+(const Vector4u& v) const;
	Vector4u& operator+=(const Vector4u& v);

	Vector4u operator-(const Vector4u& v) const;
	Vector4u& operator-=(const Vector4u& v);

	Vector4u operator*(unsigned int s) const;
	Vector4u& operator*=(unsigned int s);

	Vector4u operator/(unsigned int s) const;
	Vector4u& operator/=(unsigned int s);

	bool operator==(const Vector4u& v) const;
	bool operator!=(const Vector4u& v) const;

	unsigned int& operator[](unsigned int index);
	unsigned int operator[](unsigned int index) const;
};

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
