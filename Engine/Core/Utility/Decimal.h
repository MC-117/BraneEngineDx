#pragma once
#include <iostream>
#include <type_traits>

struct ENGINE_API Decimal
{
	enum Type : char {
		Int, Float
	} type;

	union
	{
		long long integer;
		double floatingPoint;
	} value;

	Decimal(int num);
	Decimal(long long num);
	Decimal(unsigned num);
	Decimal(unsigned long long num);
	Decimal(float num);
	Decimal(double num);

	Decimal& operator=(int num);
	Decimal& operator=(long long num);
	Decimal& operator=(unsigned num);
	Decimal& operator=(float num);
	Decimal& operator=(double num);

	operator int() const;
	operator long long() const;
	operator unsigned() const;
	operator float() const;
	operator double() const;

	std::string toString() const;
	friend std::ostream& operator<<(std::ostream& os, const Decimal& decimal);
	Decimal& parse(const std::string& str);
};