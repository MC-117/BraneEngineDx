#include "Decimal.h"
#include <string>

Decimal::Decimal(int num)
{
	type = Int;
	value.integer = num;
}

Decimal::Decimal(long long num)
{
	type = Int;
	value.integer = num;
}

Decimal::Decimal(unsigned num)
{
	type = Int;
	value.integer = num;
}

Decimal::Decimal(unsigned long long num)
{
	type = Int;
	value.integer = num;
}

Decimal::Decimal(float num)
{
	type = Float;
	value.floatingPoint = num;
}

Decimal::Decimal(double num)
{
	type = Float;
	value.floatingPoint = num;
}

Decimal& Decimal::operator=(int num)
{
	type = Int;
	value.integer = num;
	return *this;
}

Decimal& Decimal::operator=(long long num)
{
	type = Int;
	value.integer = num;
	return *this;
}

Decimal& Decimal::operator=(unsigned num)
{
	type = Int;
	value.integer = num;
	return *this;
}

Decimal& Decimal::operator=(float num)
{
	type = Float;
	value.floatingPoint = num;
	return *this;
}

Decimal& Decimal::operator=(double num)
{
	type = Float;
	value.floatingPoint = num;
	return *this;
}

Decimal::operator int() const
{
	if (type == Int)
		return value.integer;
	else
		return (int)value.floatingPoint;
}

Decimal::operator long long() const
{
	if (type == Int)
		return value.integer;
	else
		return (long long)value.floatingPoint;
}

Decimal::operator unsigned() const
{
	if (type == Int)
		return value.integer;
	else
		return (unsigned)value.floatingPoint;
}

Decimal::operator float() const
{
	if (type == Int)
		return (float)value.integer;
	else
		return value.floatingPoint;
}

Decimal::operator double() const
{
	if (type == Int)
		return (double)value.integer;
	else
		return value.floatingPoint;
}

std::string Decimal::toString() const
{
	if (type == Int)
		return std::to_string(value.integer);
	else
		return std::to_string(value.floatingPoint);
}

std::ostream& operator<<(std::ostream& os, const Decimal& decimal)
{
	switch (decimal.type) {
	case Decimal::Int: os << decimal.value.integer; break;
	case Decimal::Float: os << decimal.value.floatingPoint; break;
	}
	return os;
}

Decimal& Decimal::parse(const std::string& str)
{
	if (str.find('.') == std::string::npos &&
		str.find('e') == std::string::npos) {
		type = Float;
		value.floatingPoint = atof(str.c_str());
	}
	else {
		type = Int;
		value.integer = atoll(str.c_str());
	}
	return *this;
}
