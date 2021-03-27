#include "MathLibrary.h"
#include <iostream>

namespace dx = DirectX;
using namespace DirectX;
using namespace std;

Block::Block(float* d, unsigned int rowCount, unsigned int colCount, 
	unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum)
	: d(d), rowCount(rowCount), colCount(colCount),
	rowStart(rowStart), colStart(colStart),
	rowNum(rowNum), colNum(colNum)
{
}

void Block::validate() const
{
	if (d == NULL || (rowCount * colCount * rowNum * colNum) == 0)
		throw runtime_error("invalid block");
	if ((rowStart + rowNum) > rowCount || (colStart + colNum) > colCount)
		throw overflow_error("Out of size");
}

Block Block::row(unsigned int index)
{
	if (index >= rowNum)
		throw overflow_error("Out of size");
	return Block(d, rowCount, colCount, rowStart + index, colStart, 1, colNum);
}

Block Block::col(unsigned int index)
{
	if (index >= colNum)
		throw overflow_error("Out of size");
	return Block(d, rowCount, colCount, rowStart, colStart + index, rowNum, 1);
}

unsigned int Block::rows() const
{
	return rowNum;
}

unsigned int Block::cols() const
{
	return colNum;
}

Block Block::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum)
{
	if ((rowStart + rowNum) > this->rowNum || (colStart + colNum) > this->rowNum)
		throw overflow_error("Out of size");
	return Block(d, rowCount, colCount, rowStart + this->rowStart,
		colStart + this->colStart, rowNum + this->rowNum, colNum + this->colNum);
}

const Block Block::block(unsigned int rowStart, unsigned int colStart, unsigned int rowNum, unsigned int colNum) const
{
	if ((rowStart + rowNum) > this->rowNum || (colStart + colNum) > this->rowNum)
		throw overflow_error("Out of size");
	return Block(d, rowCount, colCount, rowStart + this->rowStart,
		colStart + this->colStart, rowNum + this->rowNum, colNum + this->colNum);
}

float& Block::operator()(unsigned int r, unsigned int c)
{
	validate();
	if (r >= rowNum || c >= colNum)
		throw overflow_error("Out of size");
	return d[(colStart + c) * colCount + rowStart + r];
}

float Block::operator()(unsigned int r, unsigned int c) const
{
	validate();
	if (r >= rowNum || c >= colNum)
		throw overflow_error("Out of size");
	return d[(colStart + c) * colCount + rowStart + r];
}

Block& Block::operator=(const Block& b)
{
	if (rowNum != b.rowNum || colNum != b.colNum)
		throw runtime_error("Incompatible Size");
	for (int i = 0; i < rowNum; i++)
		for (int j = 0; j < colNum; j++)
			(*this)(i, j) = b(i, j);
	return *this;
}

bool Block::operator==(const Block& b) const
{
	if (rowNum != b.rowNum || colNum != b.colNum)
		return false;
	for (int i = 0; i < rowNum; i++)
		for (int j = 0; j < colNum; j++)
			if ((*this)(i, j) != b(i, j))
				return false;
	return true;
}

bool Block::operator!=(const Block& b) const
{
	if (rowNum != b.rowNum || colNum != b.colNum)
		return true;
	for (int i = 0; i < rowNum; i++)
		for (int j = 0; j < colNum; j++)
			if ((*this)(i, j) != b(i, j))
				return true;
	return false;
}

Vector2f::Vector2f(float x, float y) : XMFLOAT2(x, y)
{
}

Vector2f::Vector2f(const Vector2f& v) : XMFLOAT2(v)
{
}

Vector2f::Vector2f(const Block& b)
{
	if (b.rowNum != 2 || b.colNum != 1)
		throw runtime_error("incompatible Size");
	x() = b(0, 0);
	y() = b(1, 0);
}

Vector2f::Vector2f(const XMFLOAT2& xmf2) : XMFLOAT2(xmf2)
{
}

float& Vector2f::x()
{
	return XMFLOAT2::x;
}

float Vector2f::x() const
{
	return XMFLOAT2::x;
}

float& Vector2f::y()
{
	return XMFLOAT2::y;
}

float Vector2f::y() const
{
	return XMFLOAT2::y;
}

Vector2f Vector2f::Identity()
{
	return Vector2f(1, 0);
}

Vector2f Vector2f::Zero()
{
	return Vector2f(0, 0);
}

Vector2f Vector2f::Ones()
{
	return Vector2f(1, 1);
}

Vector2f Vector2f::UnitX()
{
	return Vector2f(1, 0);
}

Vector2f Vector2f::UnitY()
{
	return Vector2f(0, 1);
}

float* Vector2f::data() const
{
	return (float*)this;
}

float Vector2f::dot(const Vector2f& v) const
{
	return x() * v.x() + y() * v.y();
}

Vector2f Vector2f::cross(const Vector2f& v) const
{
	Vector2f out;
	dx::XMStoreFloat2(&out, dx::XMVector2Cross(dx::XMLoadFloat2(this), dx::XMLoadFloat2(&v)));
	return out;
}

float Vector2f::squaredNorm() const
{
	return x() * x() + y() * y();
}

float Vector2f::norm() const
{
	return sqrt(x() * x() + y() * y());
}

Vector2f Vector2f::normalized() const
{
	float s = 1 / sqrt(x() * x() + y() * y());
	return Vector2f(x() * s, y() * s);
}

Vector2f& Vector2f::normalize()
{
	float s = 1 / sqrt(x() * x() + y() * y());
	x() *= s;
	y() *= s;
	return *this;
}

Vector2f Vector2f::cwiseProduct(const Vector2f& v) const
{
	return Vector2f(x() * v.x(), y() * v.y());
}

Block Vector2f::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum)
{
	if ((rowStart + rowNum) > 2 || (colStart + colNum) > 1)
		throw overflow_error("Out of size");
	return Block((float*)this, 2, 1, rowStart, colStart, rowNum, colNum);
}

const Block Vector2f::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum) const
{
	if ((rowStart + rowNum) > 2 || (colStart + colNum) > 1)
		throw overflow_error("Out of size");
	return Block((float*)this, 2, 1, rowStart, colStart, rowNum, colNum);
}

Vector2f& Vector2f::operator=(const Vector2f& v)
{
	XMFLOAT2::operator=(v);
	return *this;
}

Vector2f Vector2f::operator-()
{
	return Vector2f(-x(), -y());
}

Vector2f Vector2f::operator+(const Vector2f& v) const
{
	return Vector2f(x() + v.x(), y() + v.y());
}

Vector2f& Vector2f::operator+=(const Vector2f& v)
{
	x() += v.x();
	y() += v.y();
	return *this;
}

Vector2f Vector2f::operator-(const Vector2f& v) const
{
	return Vector2f(x() - v.x(), y() - v.y());
}

Vector2f& Vector2f::operator-=(const Vector2f& v)
{
	x() -= v.x();
	y() -= v.y();
	return *this;
}

Vector2f Vector2f::operator*(float s) const
{
	return Vector2f(x() * s, y() * s);
}

Vector2f& Vector2f::operator*=(float s)
{
	x() *= s;
	y() *= s;
	return *this;
}

Vector2f Vector2f::operator/(float s) const
{
	return Vector2f(x() / s, y() / s);
}

Vector2f& Vector2f::operator/=(float s)
{
	x() /= s;
	y() /= s;
	return *this;
}

bool Vector2f::operator==(const Vector2f& v) const
{
	return x() == v.x() && y() == v.y();
}

bool Vector2f::operator!=(const Vector2f& v) const
{
	return x() != v.x() || y() != v.y();
}

float& Vector2f::operator[](unsigned int index)
{
	if (index == 0)
		return x();
	else if (index == 1)
		return y();
	else
		throw overflow_error("Vector2f only has 2 elements");
}

float Vector2f::operator[](unsigned int index) const
{
	if (index == 0)
		return x();
	else if (index == 1)
		return y();
	else
		throw overflow_error("Vector2f only has 2 elements");
}

Vector2f::operator Block()
{
	return Block((float*)this, 2, 1, 0, 0, 2, 1);
}

Vector2f::operator Block() const
{
	return Block((float*)this, 2, 1, 0, 0, 2, 1);
}

Vector2u::Vector2u(unsigned int x, unsigned int y)
	: _x(x), _y(y)
{
}

Vector2u::Vector2u(const Vector2u& v)
{
	_x = v._x;
	_y = v._y;
}

unsigned int& Vector2u::x()
{
	return _x;
}

unsigned int Vector2u::x() const
{
	return _x;
}

unsigned int& Vector2u::y()
{
	return _y;
}

unsigned int Vector2u::y() const
{
	return _y;
}

Vector2u Vector2u::Identity()
{
	return Vector2u(1, 0);
}

Vector2u Vector2u::Zero()
{
	return Vector2u();
}

Vector2u Vector2u::Ones()
{
	return Vector2u(1, 1);
}

Vector2u Vector2u::UnitX()
{
	return Vector2u(1, 0);
}

Vector2u Vector2u::UnitY()
{
	return Vector2u(0, 1);
}

unsigned int* Vector2u::data() const
{
	return (unsigned int*)this;
}

float Vector2u::dot(const Vector2u& v) const
{
	return Vector2f(_x, _y).dot(Vector2f(v._x, v._y));
}

Vector2f Vector2u::cross(const Vector2u& v) const
{
	return Vector2f(_x, _y).cross(Vector2f(v._x, v._y));
}

float Vector2u::squaredNorm() const
{
	return Vector2f(_x, _y).squaredNorm();
}

float Vector2u::norm() const
{
	return Vector2f(_x, _y).norm();
}

Vector2f Vector2u::normalized() const
{
	return Vector2f(_x, _y).normalized();
}

Vector2u Vector2u::cwiseProduct(const Vector2u& v) const
{
	return Vector2u(_x * v._x, _y * v._y);
}

Vector2u& Vector2u::operator=(const Vector2u& v)
{
	_x = v._x;
	_y = v._y;
	return *this;
}

Vector2f Vector2u::operator-()
{
	return Vector2f(-(float)_x, -(float)_y);
}

Vector2u Vector2u::operator+(const Vector2u& v) const
{
	return Vector2u(_x + v._x, _y + v._y);
}

Vector2u& Vector2u::operator+=(const Vector2u& v)
{
	_x += v._x;
	_y += v._y;
	return *this;
}

Vector2u Vector2u::operator-(const Vector2u& v) const
{
	return Vector2u(_x - v._x, _y - v._y);
}

Vector2u& Vector2u::operator-=(const Vector2u& v)
{
	_x -= v._x;
	_y -= v._y;
	return *this;
}

Vector2u Vector2u::operator*(unsigned int s) const
{
	return Vector2u(_x * s, _y * s);
}

Vector2u& Vector2u::operator*=(unsigned int s)
{
	_x *= s;
	_y *= s;
	return *this;
}

Vector2u Vector2u::operator/(unsigned int s) const
{
	return Vector2u(_x / s, _y / s);
}

Vector2u& Vector2u::operator/=(unsigned int s)
{
	_x /= s;
	_y /= s;
	return *this;
}

bool Vector2u::operator==(const Vector2u& v) const
{
	return _x == v._x && _y == v._y;
}

bool Vector2u::operator!=(const Vector2u& v) const
{
	return _x != v._x || _y != v._y;
}

unsigned int& Vector2u::operator[](unsigned int index)
{
	if (index > 1)
		throw overflow_error("Vector2u only has 2 elements");
	return ((unsigned int*)this)[index];
}

unsigned int Vector2u::operator[](unsigned int index) const
{
	if (index > 1)
		throw overflow_error("Vector2u only has 2 elements");
	return ((unsigned int*)this)[index];
}

Vector3f::Vector3f(float x, float y, float z) : XMFLOAT3(x, y, z)
{
}

Vector3f::Vector3f(const Vector3f& v) : XMFLOAT3(v)
{
}

Vector3f::Vector3f(const Block& b)
{
	if (b.rowNum != 3 || b.colNum != 1)
		throw runtime_error("incompatible Size");
	x() = b(0, 0);
	y() = b(1, 0);
	z() = b(2, 0);
}

Vector3f::Vector3f(const XMFLOAT3& xmf3) : XMFLOAT3(xmf3)
{
}

float& Vector3f::x()
{
	return XMFLOAT3::x;
}

float Vector3f::x() const
{
	return XMFLOAT3::x;
}

float& Vector3f::y()
{
	return XMFLOAT3::y;
}

float Vector3f::y() const
{
	return XMFLOAT3::y;
}

float& Vector3f::z()
{
	return XMFLOAT3::z;
}

float Vector3f::z() const
{
	return XMFLOAT3::z;
}

Vector3f Vector3f::Identity()
{
	return Vector3f(1, 0, 0);
}

Vector3f Vector3f::Zero()
{
	return Vector3f(0, 0, 0);
}

Vector3f Vector3f::Ones()
{
	return Vector3f(1, 1, 1);
}

Vector3f Vector3f::UnitX()
{
	return Vector3f(1, 0, 0);
}

Vector3f Vector3f::UnitY()
{
	return Vector3f(0, 1, 0);
}

Vector3f Vector3f::UnitZ()
{
	return Vector3f(0, 0, 1);
}

float* Vector3f::data() const
{
	return (float*)this;
}

float Vector3f::dot(const Vector3f& v) const
{
	return x() * v.x() + y() * v.y() + z() * v.z();
}

Vector3f Vector3f::cross(const Vector3f& v) const
{
	Vector3f out;
	dx::XMStoreFloat3(&out, dx::XMVector3Cross(dx::XMLoadFloat3(this), dx::XMLoadFloat3(&v)));
	return out;
}

float Vector3f::squaredNorm() const
{
	return x() * x() + y() * y() + z() * z();
}

float Vector3f::norm() const
{
	return sqrt(x() * x() + y() * y() + z() * z());
}

Vector3f Vector3f::normalized() const
{
	float s = 1 / sqrt(x() * x() + y() * y() + z() * z());
	return Vector3f(x() * s, y() * s, z() * s);
}

Vector3f& Vector3f::normalize()
{
	float s = 1 / sqrt(x() * x() + y() * y() + z() * z());
	x() *= s;
	y() *= s;
	z() *= s;
	return *this;
}

Vector3f Vector3f::cwiseProduct(const Vector3f& v) const
{
	return Vector3f(x() * v.x(), y() * v.y(), z() * v.z());
}

Block Vector3f::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum)
{
	if ((rowStart + rowNum) > 3 || (colStart + colNum) > 1)
		throw overflow_error("Out of size");
	return Block((float*)this, 3, 1, rowStart, colStart, rowNum, colNum);
}

const Block Vector3f::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum) const
{
	if ((rowStart + rowNum) > 3 || (colStart + colNum) > 1)
		throw overflow_error("Out of size");
	return Block((float*)this, 3, 1, rowStart, colStart, rowNum, colNum);
}

Vector3f& Vector3f::operator=(const Vector3f& v)
{
	XMFLOAT3::operator=(v);
	return *this;
}

Vector3f Vector3f::operator-()
{
	return Vector3f(-x(), -y(), -z());
}

Vector3f Vector3f::operator+(const Vector3f& v) const
{
	return Vector3f(x() + v.x(), y() + v.y(), z() + v.z());
}

Vector3f& Vector3f::operator+=(const Vector3f& v)
{
	x() += v.x();
	y() += v.y();
	z() += v.z();
	return *this;
}

Vector3f Vector3f::operator-(const Vector3f& v) const
{
	return Vector3f(x() - v.x(), y() - v.y(), z() - v.z());
}

Vector3f& Vector3f::operator-=(const Vector3f& v)
{
	x() -= v.x();
	y() -= v.y();
	z() -= v.z();
	return *this;
}

Vector3f Vector3f::operator*(float s) const
{
	return Vector3f(x() * s, y() * s, z() * s);
}

Vector3f& Vector3f::operator*=(float s)
{
	x() *= s;
	y() *= s;
	z() *= s;
	return *this;
}

Vector3f Vector3f::operator/(float s) const
{
	return Vector3f(x() / s, y() / s, z() / s);
}

Vector3f& Vector3f::operator/=(float s)
{
	x() /= s;
	y() /= s;
	z() /= s;
	return *this;
}

bool Vector3f::operator==(const Vector3f& v) const
{
	return x() == v.x() && y() == v.y() && z() == v.z();
}

bool Vector3f::operator!=(const Vector3f& v) const
{
	return x() != v.x() || y() != v.y() || z() != v.z();
}

float& Vector3f::operator[](unsigned int index)
{
	if (index > 2)
		throw overflow_error("Vector3f only has 3 elements");
	return ((float*)this)[index];
}

float Vector3f::operator[](unsigned int index) const
{
	if (index > 2)
		throw overflow_error("Vector3f only has 3 elements");
	return ((float*)this)[index];
}

Vector3f::operator Block()
{
	return Block((float*)this, 3, 1, 0, 0, 3, 1);
}

Vector3f::operator Block() const
{
	return Block((float*)this, 3, 1, 0, 0, 3, 1);
}

Vector3u::Vector3u(unsigned int x, unsigned int y, unsigned int z)
	: _x(x), _y(y), _z(z)
{
}

Vector3u::Vector3u(const Vector3u& v)
{
	_x = v._x;
	_y = v._y;
	_z = v._z;
}

unsigned int& Vector3u::x()
{
	return _x;
}

unsigned int Vector3u::x() const
{
	return _x;
}

unsigned int& Vector3u::y()
{
	return _y;
}

unsigned int Vector3u::y() const
{
	return _y;
}

unsigned int& Vector3u::z()
{
	return _z;
}

unsigned int Vector3u::z() const
{
	return _z;
}

Vector3u Vector3u::Identity()
{
	return Vector3u(1, 0, 0);
}

Vector3u Vector3u::Zero()
{
	return Vector3u();
}

Vector3u Vector3u::Ones()
{
	return Vector3u(1, 1, 1);
}

Vector3u Vector3u::UnitX()
{
	return Vector3u(1, 0, 0);
}

Vector3u Vector3u::UnitY()
{
	return Vector3u(0, 1, 0);
}

Vector3u Vector3u::UnitZ()
{
	return Vector3u(0, 0, 1);
}

unsigned int* Vector3u::data() const
{
	return (unsigned int*)this;
}

float Vector3u::dot(const Vector3u& v) const
{
	return Vector3f(_x, _y, _z).dot(Vector3f(v._x, v._y, v._z));
}

Vector3f Vector3u::cross(const Vector3u& v) const
{
	return Vector3f(_x, _y, _z).cross(Vector3f(v._x, v._y, v._z));
}

float Vector3u::squaredNorm() const
{
	return Vector3f(_x, _y, _z).squaredNorm();
}

float Vector3u::norm() const
{
	return Vector3f(_x, _y, _z).norm();
}

Vector3f Vector3u::normalized() const
{
	return Vector3f(_x, _y, _z).normalized();
}

Vector3u Vector3u::cwiseProduct(const Vector3u& v) const
{
	return Vector3u(_x * v._x, _y * v._y, _z * v._z);
}

Vector3u& Vector3u::operator=(const Vector3u& v)
{
	_x = v._x;
	_y = v._y;
	_z = v._z;
	return *this;
}

Vector3f Vector3u::operator-()
{
	return Vector3f(-(float)_x, -(float)_y, -(float)_z);
}

Vector3u Vector3u::operator+(const Vector3u& v) const
{
	return Vector3u(_x + v._x, _y + v._y, _z + v._z);
}

Vector3u& Vector3u::operator+=(const Vector3u& v)
{
	_x += v._x;
	_y += v._y;
	_z += v._z;
	return *this;
}

Vector3u Vector3u::operator-(const Vector3u& v) const
{
	return Vector3u(_x - v._x, _y - v._y, _z - v._z);
}

Vector3u& Vector3u::operator-=(const Vector3u& v)
{
	_x -= v._x;
	_y -= v._y;
	_z -= v._z;
	return *this;
}

Vector3u Vector3u::operator*(unsigned int s) const
{
	return Vector3u(_x * s, _y * s, _z * s);
}

Vector3u& Vector3u::operator*=(unsigned int s)
{
	_x *= s;
	_y *= s;
	_z *= s;
	return *this;
}

Vector3u Vector3u::operator/(unsigned int s) const
{
	return Vector3u(_x / s, _y / s, _z / s);
}

Vector3u& Vector3u::operator/=(unsigned int s)
{
	_x /= s;
	_y /= s;
	_z /= s;
	return *this;
}

bool Vector3u::operator==(const Vector3u& v) const
{
	return _x == v._x && _y == v._y && _z == v._z;
}

bool Vector3u::operator!=(const Vector3u& v) const
{
	return _x != v._x || _y != v._y || _z != v._z;
}

unsigned int& Vector3u::operator[](unsigned int index)
{
	if (index > 2)
		throw overflow_error("Vector3u only has 3 elements");
	return ((unsigned int*)this)[index];
}

unsigned int Vector3u::operator[](unsigned int index) const
{
	if (index > 2)
		throw overflow_error("Vector3u only has 3 elements");
	return ((unsigned int*)this)[index];
}

Vector4f::Vector4f(float x, float y, float z, float w) : XMFLOAT4(x, y, z, w)
{
}

Vector4f::Vector4f(const Vector4f& v) : XMFLOAT4(v)
{
}

Vector4f::Vector4f(const Block& b)
{
	if (b.rowNum != 4 || b.colNum != 1)
		throw runtime_error("incompatible Size");
	x() = b(0, 0);
	y() = b(1, 0);
	z() = b(2, 0);
	w() = b(3, 0);
}

Vector4f::Vector4f(const XMFLOAT4& xmf4) : XMFLOAT4(xmf4)
{
}

float& Vector4f::x()
{
	return XMFLOAT4::x;
}

float Vector4f::x() const
{
	return XMFLOAT4::x;
}

float& Vector4f::y()
{
	return XMFLOAT4::y;
}

float Vector4f::y() const
{
	return XMFLOAT4::y;
}

float& Vector4f::z()
{
	return XMFLOAT4::z;
}

float Vector4f::z() const
{
	return XMFLOAT4::z;
}

float& Vector4f::w()
{
	return XMFLOAT4::w;
}

float Vector4f::w() const
{
	return XMFLOAT4::w;
}

Vector4f Vector4f::Identity()
{
	return Vector4f(1, 0, 0, 0);
}

Vector4f Vector4f::Zero()
{
	return Vector4f(0, 0, 0, 0);
}

Vector4f Vector4f::Ones()
{
	return Vector4f(1, 1, 1, 1);
}

Vector4f Vector4f::UnitX()
{
	return Vector4f(1, 0, 0, 0);
}

Vector4f Vector4f::UnitY()
{
	return Vector4f(0, 1, 0, 0);
}

Vector4f Vector4f::UnitZ()
{
	return Vector4f(0, 0, 1, 0);
}

Vector4f Vector4f::UnitW()
{
	return Vector4f(0, 0, 0, 1);
}

float* Vector4f::data() const
{
	return (float*)this;
}

float Vector4f::dot(const Vector4f& v) const
{
	return x() * v.x() + y() * v.y() + z() * v.z() + w() * v.w();
}

float Vector4f::squaredNorm() const
{
	return x() * x() + y() * y() + z() * z() + w() * w();
}

float Vector4f::norm() const
{
	return sqrt(x() * x() + y() * y() + z() * z() + w() * w());
}

Vector4f Vector4f::normalized() const
{
	float s = 1 / sqrt(x() * x() + y() * y() + z() * z() + w() * w());
	return Vector4f(x() * s, y() * s, z() * s, w() * s);
}

Vector4f& Vector4f::normalize()
{
	float s = 1 / sqrt(x() * x() + y() * y() + z() * z() + w() * w());
	x() *= s;
	y() *= s;
	z() *= s;
	w() *= s;
	return *this;
}

Vector4f Vector4f::cwiseProduct(const Vector4f& v) const
{
	return Vector4f(x() * v.x(), y() * v.y(), z() * v.z(), w() * v.w());
}

Block Vector4f::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum)
{
	if ((rowStart + rowNum) > 4 || (colStart + colNum) > 1)
		throw overflow_error("Out of size");
	return Block((float*)this, 4, 1, rowStart, colStart, rowNum, colNum);
}

const Block Vector4f::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum) const
{
	if ((rowStart + rowNum) > 4 || (colStart + colNum) > 1)
		throw overflow_error("Out of size");
	return Block((float*)this, 4, 1, rowStart, colStart, rowNum, colNum);
}

Vector4f& Vector4f::operator=(const Vector4f& v)
{
	XMFLOAT4::operator=(v);
	return *this;
}

Vector4f Vector4f::operator-()
{
	return Vector4f(-x(), -y(), -z(), -w());
}

Vector4f Vector4f::operator+(const Vector4f& v) const
{
	return Vector4f(x() + v.x(), y() + v.y(), z() + v.z(), w() + v.w());
}

Vector4f& Vector4f::operator+=(const Vector4f& v)
{
	x() += v.x();
	y() += v.y();
	z() += v.z();
	w() += v.w();
	return *this;
}

Vector4f Vector4f::operator-(const Vector4f& v) const
{
	return Vector4f(x() - v.x(), y() - v.y(), z() - v.z(), w() - v.w());
}

Vector4f& Vector4f::operator-=(const Vector4f& v)
{
	x() -= v.x();
	y() -= v.y();
	z() -= v.z();
	w() -= v.w();
	return *this;
}

Vector4f Vector4f::operator*(float s) const
{
	return Vector4f(x() * s, y() * s, z() * s, w() * s);
}

Vector4f& Vector4f::operator*=(float s)
{
	x() *= s;
	y() *= s;
	z() *= s;
	w() *= s;
	return *this;
}

Vector4f Vector4f::operator/(float s) const
{
	return Vector4f(x() / s, y() / s, z() / s, w() / s);
}

Vector4f& Vector4f::operator/=(float s)
{
	x() /= s;
	y() /= s;
	z() /= s;
	w() /= s;
	return *this;
}

bool Vector4f::operator==(const Vector4f& v) const
{
	return x() == v.x() && y() == v.y() && z() == v.z() && w() == v.w();
}

bool Vector4f::operator!=(const Vector4f& v) const
{
	return x() != v.x() || y() != v.y() || z() != v.z() || w() != v.w();
}

float& Vector4f::operator[](unsigned int index)
{
	if (index > 3)
		throw overflow_error("Vector4f only has 4 elements");
	return ((float*)this)[index];
}

float Vector4f::operator[](unsigned int index) const
{
	if (index > 3)
		throw overflow_error("Vector4f only has 4 elements");
	return ((float*)this)[index];
}

Vector4f::operator Block()
{
	return Block((float*)this, 4, 1, 0, 0, 4, 1);
}

Vector4f::operator Block() const
{
	return Block((float*)this, 4, 1, 0, 0, 4, 1);
}

Vector4u::Vector4u(unsigned int x, unsigned int y, unsigned int z, unsigned int w)
	: _x(x), _y(y), _z(z), _w(w)
{
}

Vector4u::Vector4u(const Vector4u& v)
{
	_x = v._x;
	_y = v._y;
	_z = v._z;
	_w = v._w;
}

unsigned int& Vector4u::x()
{
	return _x;
}

unsigned int Vector4u::x() const
{
	return _x;
}

unsigned int& Vector4u::y()
{
	return _y;
}

unsigned int Vector4u::y() const
{
	return _y;
}

unsigned int& Vector4u::z()
{
	return _z;
}

unsigned int Vector4u::z() const
{
	return _z;
}

unsigned int& Vector4u::w()
{
	return _w;
}

unsigned int Vector4u::w() const
{
	return _w;
}

Vector4u Vector4u::Identity()
{
	return Vector4u(1, 0, 0, 0);
}

Vector4u Vector4u::Zero()
{
	return Vector4u();
}

Vector4u Vector4u::Ones()
{
	return Vector4u(1, 1, 1, 1);
}

Vector4u Vector4u::UnitX()
{
	return Vector4u(1, 0, 0, 0);
}

Vector4u Vector4u::UnitY()
{
	return Vector4u(0, 1, 0, 0);
}

Vector4u Vector4u::UnitZ()
{
	return Vector4u(0, 0, 1, 0);
}

Vector4u Vector4u::UnitW()
{
	return Vector4u(0, 0, 0, 1);
}

unsigned int* Vector4u::data() const
{
	return (unsigned int*)this;
}

float Vector4u::dot(const Vector4u& v) const
{
	return Vector4f(_x, _y, _z, _w).dot(Vector4f(v._x, v._y, v._z, _w));
}

float Vector4u::squaredNorm() const
{
	return Vector4f(_x, _y, _z, _w).squaredNorm();
}

float Vector4u::norm() const
{
	return Vector4f(_x, _y, _z, _w).norm();
}

Vector4f Vector4u::normalized() const
{
	return Vector4f(_x, _y, _z, _w).normalized();
}

Vector4u Vector4u::cwiseProduct(const Vector4u& v) const
{
	return Vector4u(_x * v._x, _y * v._y, _z * v._z, _w * v._w);
}

Vector4u& Vector4u::operator=(const Vector4u& v)
{
	_x = v._x;
	_y = v._y;
	_z = v._z;
	_w = v._w;
	return *this;
}

Vector4f Vector4u::operator-()
{
	return Vector4f(-(float)_x, -(float)_y, -(float)_z, -(float)_w);
}

Vector4u Vector4u::operator+(const Vector4u& v) const
{
	return Vector4u(_x + v._x, _y + v._y, _z + v._z, _w + v._w);
}

Vector4u& Vector4u::operator+=(const Vector4u& v)
{
	_x += v._x;
	_y += v._y;
	_z += v._z;
	_w += v._w;
	return *this;
}

Vector4u Vector4u::operator-(const Vector4u& v) const
{
	return Vector4u(_x - v._x, _y - v._y, _z - v._z, _w - v._w);
}

Vector4u& Vector4u::operator-=(const Vector4u& v)
{
	_x -= v._x;
	_y -= v._y;
	_z -= v._z;
	_w -= v._w;
	return *this;
}

Vector4u Vector4u::operator*(unsigned int s) const
{
	return Vector4u(_x * s, _y * s, _z * s, _w * s);
}

Vector4u& Vector4u::operator*=(unsigned int s)
{
	_x *= s;
	_y *= s;
	_z *= s;
	_w *= s;
	return *this;
}

Vector4u Vector4u::operator/(unsigned int s) const
{
	return Vector4u(_x / s, _y / s, _z / s, _w / s);
}

Vector4u& Vector4u::operator/=(unsigned int s)
{
	_x /= s;
	_y /= s;
	_z /= s;
	_w /= s;
	return *this;
}

bool Vector4u::operator==(const Vector4u& v) const
{
	return _x == v._x && _y == v._y && _z == v._z && _w == v._w;
}

bool Vector4u::operator!=(const Vector4u& v) const
{
	return _x != v._x || _y != v._y || _z != v._z || _w != v._w;
}

unsigned int& Vector4u::operator[](unsigned int index)
{
	if (index > 3)
		throw overflow_error("Vector4u only has 4 elements");
	return ((unsigned int*)this)[index];
}

unsigned int Vector4u::operator[](unsigned int index) const
{
	if (index > 3)
		throw overflow_error("Vector4u only has 4 elements");
	return ((unsigned int*)this)[index];
}

Matrix3f::Matrix3f() : XMFLOAT3X3()
{
}

Matrix3f::Matrix3f(const Matrix3f& m) : XMFLOAT3X3(m)
{
}

Matrix3f::Matrix3f(const float* pArray) : XMFLOAT3X3(pArray)
{
}

Matrix3f::Matrix3f(const Block& b)
{
	Block _b((float*)this, 3, 3, 0, 0, 3, 3);
	_b = b;
}

Matrix3f::Matrix3f(const XMFLOAT3X3& xmf3x3) : XMFLOAT3X3(xmf3x3)
{
}

Matrix3f::Matrix3f(const XMMATRIX& xmm)
{
	dx::XMStoreFloat3x3(this, xmm);
}

Matrix3f Matrix3f::Identity()
{
	return XMFLOAT3X3(
		1.f, 0.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 0.f, 1.f
	);
}

Matrix3f Matrix3f::Zero()
{
	return XMFLOAT3X3(
		0.f, 0.f, 0.f,
		0.f, 0.f, 0.f,
		0.f, 0.f, 0.f
	);
}

float* Matrix3f::data() const
{
	return (float*)this;
}

Matrix3f Matrix3f::inverse() const
{
	XMFLOAT3X3 out;
	dx::XMStoreFloat3x3(&out, dx::XMMatrixInverse(NULL, dx::XMLoadFloat3x3(this)));
	return out;
}

Matrix3f Matrix3f::transpose() const
{
	XMFLOAT3X3 out;
	dx::XMStoreFloat3x3(&out, dx::XMMatrixTranspose(dx::XMLoadFloat3x3(this)));
	return out;
}

Matrix3f& Matrix3f::transposeInPlace()
{
	dx::XMStoreFloat3x3(this, dx::XMMatrixTranspose(dx::XMLoadFloat3x3(this)));
	return *this;
}

Vector3f Matrix3f::eulerAngles() const
{
	float ox, oy, oz;
	ox = atan2((*this)(1, 2), (*this)(2, 2));
	float m00 = (*this)(0, 0), m01 = (*this)(0, 1), m02 = (*this)(0, 2);
	float d = sqrt(m00 * m00 + m01 * m01);
	if (ox > 0) {
		ox -= 3.141592653589793238462643383279502884197169399375105820974944592307816406L;
		oy = atan2(-m02, -d);
	}
	else
		oy = atan2(-m02, d);
	float sinX = sin(ox);
	float cosX = cos(ox);
	oz = atan2(sinX * (*this)(2, 0) - cosX * (*this)(1, 0),
		cosX * (*this)(1, 1) - sinX * (*this)(2, 1));
	return Vector3f(-ox, -oy, -oz);
}

Block Matrix3f::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum)
{
	if ((rowStart + rowNum) > 3 || (colStart + colNum) > 3)
		throw overflow_error("Out of size");
	return Block((float*)this, 3, 3, rowStart, colStart, rowNum, colNum);
}

const Block Matrix3f::block(unsigned int rowStart, unsigned int colStart,
	unsigned int rowNum, unsigned int colNum) const
{
	if ((rowStart + rowNum) > 3 || (colStart + colNum) > 3)
		throw overflow_error("Out of size");
	return Block((float*)this, 3, 3, rowStart, colStart, rowNum, colNum);
}

float& Matrix3f::operator()(unsigned int row, unsigned int col)
{
	return XMFLOAT3X3::operator()(col, row);
}

float Matrix3f::operator()(unsigned int row, unsigned int col) const
{
	return XMFLOAT3X3::operator()(col, row);
}

Matrix3f& Matrix3f::operator=(const Matrix3f& m)
{
	XMFLOAT3X3::operator=(m);
	return *this;
}

Matrix3f Matrix3f::operator*(float s) const
{
	XMVECTOR ss = _mm_load_ss(&s);
	ss = XM_PERMUTE_PS(ss, _MM_SHUFFLE(0, 0, 0, 0));
	XMMATRIX xmm = dx::XMLoadFloat3x3(this);
	xmm.r[0] = _mm_mul_ps(xmm.r[0], ss);
	xmm.r[1] = _mm_mul_ps(xmm.r[1], ss);
	xmm.r[2] = _mm_mul_ps(xmm.r[2], ss);
	XMFLOAT3X3 out;
	dx::XMStoreFloat3x3(&out, xmm);
	return out;
}

Matrix3f& Matrix3f::operator*=(float s)
{
	XMVECTOR ss = _mm_load_ss(&s);
	ss = XM_PERMUTE_PS(ss, _MM_SHUFFLE(0, 0, 0, 0));
	XMMATRIX xmm = dx::XMLoadFloat3x3(this);
	xmm.r[0] = _mm_mul_ps(xmm.r[0], ss);
	xmm.r[1] = _mm_mul_ps(xmm.r[1], ss);
	xmm.r[2] = _mm_mul_ps(xmm.r[2], ss);
	dx::XMStoreFloat3x3(this, xmm);
	return *this;
}

Matrix3f Matrix3f::operator*(const Quaternionf& q) const
{
	XMFLOAT3X3 out;
	dx::XMStoreFloat3x3(&out, dx::XMMatrixMultiply(dx::XMMatrixRotationQuaternion(dx::XMLoadFloat4((XMFLOAT4*)&q)), dx::XMLoadFloat3x3(this)));
	return out;
}

Matrix3f& Matrix3f::operator*(const Quaternionf& q)
{
	dx::XMStoreFloat3x3(this, dx::XMMatrixMultiply(dx::XMMatrixRotationQuaternion(dx::XMLoadFloat4((XMFLOAT4*)&q)), dx::XMLoadFloat3x3(this)));
	return *this;
}

Vector3f Matrix3f::operator*(const Vector3f& v) const
{
	XMFLOAT3 out;
	dx::XMStoreFloat3(&out, dx::XMVector3Transform(dx::XMLoadFloat3((XMFLOAT3*)&v), dx::XMLoadFloat3x3(this)));
	return out;
}

Matrix3f Matrix3f::operator*(const Matrix3f& m) const
{
	XMFLOAT3X3 out;
	dx::XMStoreFloat3x3(&out, dx::XMMatrixMultiply(dx::XMLoadFloat3x3(&m), dx::XMLoadFloat3x3(this)));
	return out;
}

Matrix3f& Matrix3f::operator*=(const Matrix3f& m)
{
	dx::XMStoreFloat3x3(this, dx::XMMatrixMultiply(dx::XMLoadFloat3x3(&m), dx::XMLoadFloat3x3(this)));
	return *this;
}

Matrix3f::operator Block()
{
	return Block((float*)this, 3, 3, 0, 0, 3, 3);
}

Matrix3f::operator Block() const
{
	return Block((float*)this, 3, 3, 0, 0, 3, 3);
}

Matrix4f::Matrix4f() : XMFLOAT4X4()
{
}

Matrix4f::Matrix4f(const Matrix4f& m) : XMFLOAT4X4(m)
{
}

Matrix4f::Matrix4f(const float* pArray) : XMFLOAT4X4(pArray)
{
}

Matrix4f::Matrix4f(const Block& b)
{
	Block _b((float*)this, 4, 4, 0, 0, 4, 4);
	_b = b;
}

Matrix4f::Matrix4f(const XMFLOAT4X4& xmf4x4) : XMFLOAT4X4(xmf4x4)
{
}

Matrix4f::Matrix4f(const XMMATRIX& xmm)
{
	dx::XMStoreFloat4x4(this, xmm);
}

Matrix4f Matrix4f::Identity()
{
	return XMFLOAT4X4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
}

Matrix4f Matrix4f::Zero()
{
	return XMFLOAT4X4(
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f
	);
}

float* Matrix4f::data() const
{
	return (float*)this;
}

Matrix4f Matrix4f::inverse() const
{
	XMFLOAT4X4 out;
	dx::XMStoreFloat4x4(&out, dx::XMMatrixInverse(NULL, dx::XMLoadFloat4x4(this)));
	return out;
}

Matrix4f Matrix4f::transpose() const
{
	XMFLOAT4X4 out;
	dx::XMStoreFloat4x4(&out, dx::XMMatrixTranspose(dx::XMLoadFloat4x4(this)));
	return out;
}

Matrix4f& Matrix4f::transposeInPlace()
{
	dx::XMStoreFloat4x4(this, dx::XMMatrixTranspose(dx::XMLoadFloat4x4(this)));
	return *this;
}

bool Matrix4f::decompose(Vector3f& pos, Quaternionf& rot, Vector3f& sca) const
{
	XMVECTOR _sca, _rot, _pos;
	if (dx::XMMatrixDecompose(&_sca, &_rot, &_pos, dx::XMLoadFloat4x4(this))) {
		dx::XMStoreFloat3((XMFLOAT3*)&pos, _pos);
		dx::XMStoreFloat4((XMFLOAT4*)&rot, _rot);
		dx::XMStoreFloat3((XMFLOAT3*)&sca, _sca);
		return true;
	}
	return false;
}

Block Matrix4f::block(unsigned int rowStart, unsigned int colStart, unsigned int rowNum, unsigned int colNum)
{
	if ((rowStart + rowNum) > 4 || (colStart + colNum) > 4)
		throw overflow_error("Out of size");
	return Block((float*)this, 4, 4, rowStart, colStart, rowNum, colNum);
}

const Block Matrix4f::block(unsigned int rowStart, unsigned int colStart, unsigned int rowNum, unsigned int colNum) const
{
	if ((rowStart + rowNum) > 4 || (colStart + colNum) > 4)
		throw overflow_error("Out of size");
	return Block((float*)this, 4, 4, rowStart, colStart, rowNum, colNum);
}

float& Matrix4f::operator()(unsigned int row, unsigned int col)
{
	return XMFLOAT4X4::operator()(col, row);
}

float Matrix4f::operator()(unsigned int row, unsigned int col) const
{
	return XMFLOAT4X4::operator()(col, row);
}

Matrix4f& Matrix4f::operator=(const Matrix4f& m)
{
	XMFLOAT4X4::operator=(m);
	return *this;
}

Matrix4f Matrix4f::operator*(float s) const
{
	XMVECTOR ss = _mm_load_ss(&s);
	ss = XM_PERMUTE_PS(ss, _MM_SHUFFLE(0, 0, 0, 0));
	XMMATRIX xmm = dx::XMLoadFloat4x4(this);
	xmm.r[0] = _mm_mul_ps(xmm.r[0], ss);
	xmm.r[1] = _mm_mul_ps(xmm.r[1], ss);
	xmm.r[2] = _mm_mul_ps(xmm.r[2], ss);
	xmm.r[3] = _mm_mul_ps(xmm.r[3], ss);
	XMFLOAT4X4 out;
	dx::XMStoreFloat4x4(&out, xmm);
	return out;
}

Matrix4f& Matrix4f::operator*=(float s)
{
	XMVECTOR ss = _mm_load_ss(&s);
	ss = XM_PERMUTE_PS(ss, _MM_SHUFFLE(0, 0, 0, 0));
	XMMATRIX xmm = dx::XMLoadFloat4x4(this);
	xmm.r[0] = _mm_mul_ps(xmm.r[0], ss);
	xmm.r[1] = _mm_mul_ps(xmm.r[1], ss);
	xmm.r[2] = _mm_mul_ps(xmm.r[2], ss);
	xmm.r[3] = _mm_mul_ps(xmm.r[3], ss);
	dx::XMStoreFloat4x4(this, xmm);
	return *this;
}

Vector4f Matrix4f::operator*(const Vector4f& v) const
{
	XMFLOAT4 out;
	dx::XMStoreFloat4(&out, dx::XMVector4Transform(dx::XMLoadFloat4((XMFLOAT4*)&v), dx::XMLoadFloat4x4(this)));
	return out;
}

Matrix4f Matrix4f::operator*(const Matrix4f& m) const
{
	XMFLOAT4X4 out;
	dx::XMStoreFloat4x4(&out, dx::XMMatrixMultiply(dx::XMLoadFloat4x4(&m), dx::XMLoadFloat4x4(this)));
	return out;
}

Matrix4f& Matrix4f::operator*=(const Matrix4f& m)
{
	dx::XMStoreFloat4x4(this, dx::XMMatrixMultiply(dx::XMLoadFloat4x4(&m), dx::XMLoadFloat4x4(this)));
	return *this;
}

Matrix4f::operator Block()
{
	return Block((float*)this, 4, 4, 0, 0, 4, 4);
}

Matrix4f::operator Block() const
{
	return Block((float*)this, 4, 4, 0, 0, 4, 4);
}

Quaternionf::Quaternionf(float w, float x, float y, float z) : Vector4f(x, y, z, w)
{
}

Quaternionf::Quaternionf(const Quaternionf& q) : Vector4f(q)
{
}

Quaternionf::Quaternionf(const XMFLOAT4& xmf4) : Vector4f(xmf4)
{
}

Quaternionf::Quaternionf(const Matrix3f& m)
{
	dx::XMStoreFloat4(this, dx::XMQuaternionRotationMatrix(dx::XMLoadFloat3x3((XMFLOAT3X3*)&m)));
}

Quaternionf Quaternionf::Identity()
{
	return Quaternionf(1, 0, 0, 0);
}

Quaternionf Quaternionf::FromEularAngles(const Vector3f& rollPicthYaw)
{
	return Quaternionf().setFromEularAngles(rollPicthYaw);
}

Quaternionf Quaternionf::FromTwoVectors(const Vector3f& a, const Vector3f& b)
{
	return Quaternionf().setFromTwoVectors(a, b);
}

Quaternionf Quaternionf::FromAngleAxis(float angle, const Vector3f& axis)
{
	return Quaternionf().setFromAngleAxis(angle, axis);
}

Quaternionf Quaternionf::normalized() const
{
	XMFLOAT4 out;
	dx::XMStoreFloat4(&out, dx::XMQuaternionNormalize(dx::XMLoadFloat4(this)));
	return out;
}

Quaternionf& Quaternionf::normalize()
{
	dx::XMStoreFloat4(this, dx::XMQuaternionNormalize(dx::XMLoadFloat4(this)));
	return *this;
}

Quaternionf& Quaternionf::setIdentity()
{
	x() = 0;
	y() = 0;
	z() = 0;
	w() = 1;
	return *this;
}

Quaternionf& Quaternionf::setFromEularAngles(const Vector3f& rollPicthYaw)
{
	dx::XMStoreFloat4(this, dx::XMQuaternionRotationRollPitchYawFromVector(dx::XMLoadFloat3((XMFLOAT3*)&rollPicthYaw)));
	return *this;
}

Quaternionf& Quaternionf::setFromTwoVectors(const Vector3f& a, const Vector3f& b)
{
	Vector3f va = a.normalized();
	Vector3f vb = b.normalized();
	Vector3f c = va.cross(vb);
	float s = sqrt((1 + va.dot(vb)) * 2);
	c *= 1 / s;
	memcpy(this, &c, sizeof(float) * 3);
	w() = 0.5 * s;
	return *this;
}

Quaternionf& Quaternionf::setFromAngleAxis(float angle, const Vector3f& axis)
{
	dx::XMStoreFloat4(this, dx::XMQuaternionRotationAxis(dx::XMLoadFloat3((XMFLOAT3*)&axis), angle));
	return *this;
}

Quaternionf Quaternionf::inverse() const
{
	XMFLOAT4 out;
	dx::XMStoreFloat4(&out, dx::XMQuaternionInverse(dx::XMLoadFloat4(this)));
	return out;
}

Quaternionf Quaternionf::slerp(float t, const Quaternionf& q) const
{
	XMFLOAT4 out;
	dx::XMStoreFloat4(&out, dx::XMQuaternionSlerp(dx::XMLoadFloat4(this), dx::XMLoadFloat4(&q), t));
	return out;
}

Matrix3f Quaternionf::toRotationMatrix() const
{
	XMFLOAT3X3 out;
	dx::XMStoreFloat3x3(&out, dx::XMMatrixRotationQuaternion(dx::XMLoadFloat4(this)));
	return out;
}

Quaternionf& Quaternionf::operator=(const Quaternionf& q)
{
	Vector4f::operator=(q);
	return *this;
}

Quaternionf& Quaternionf::operator=(const Matrix3f& m)
{
	dx::XMStoreFloat4(this, dx::XMQuaternionRotationMatrix(dx::XMLoadFloat3x3((XMFLOAT3X3*)&m)));
	return *this;
}

Vector3f Quaternionf::operator*(const Vector3f& v) const
{
	XMFLOAT3 out;
	dx::XMStoreFloat3(&out, dx::XMVector3Rotate(dx::XMLoadFloat3((XMFLOAT3*)&v), dx::XMLoadFloat4(this)));
	return out;
}

Quaternionf Quaternionf::operator*(const Quaternionf& q) const
{
	XMFLOAT4 out;
	dx::XMStoreFloat4(&out, dx::XMQuaternionMultiply(dx::XMLoadFloat4(&q), dx::XMLoadFloat4(this)));
	return out;
}

Quaternionf& Quaternionf::operator*=(const Quaternionf& q)
{
	dx::XMStoreFloat4(this, dx::XMQuaternionMultiply(dx::XMLoadFloat4(&q), dx::XMLoadFloat4(this)));
	return *this;
}

Quaternionf Quaternionf::operator*(float s) const
{
	Matrix4f mat = dx::XMMatrixRotationQuaternion(dx::XMLoadFloat4(this));
	mat *= s;
	XMFLOAT4 out;
	dx::XMStoreFloat4(&out, dx::XMQuaternionRotationMatrix(dx::XMLoadFloat4x4((XMFLOAT4X4*)&mat)));
	return out;
}

Quaternionf& Quaternionf::operator*=(const float& s)
{
	Matrix4f mat = dx::XMMatrixRotationQuaternion(*(XMVECTOR*)this);
	mat *= s;
	dx::XMStoreFloat4(this, dx::XMQuaternionRotationMatrix(dx::XMLoadFloat4x4((XMFLOAT4X4*)&mat)));
	return *this;
}
