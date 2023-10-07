#pragma once
#ifndef _UNIT_H_
#define _UNIT_H_

#include "Config.h"
#include "Utility/Guid.h"

typedef unsigned TypeID;
typedef unsigned FileID;
typedef long long InstanceID;

struct InstanceAssetHandle
{
	Guid guid;
	FileID fileID;
};

struct InstanceAssetFile
{
	FileID fileID;
	InstanceID highestLocalID;
	std::string path;
};

template<class T>
struct Unit2D
{
	T x, y;

	static Unit2D<T> create(T v);
	static Unit2D<T> create(T x, T y);
	double length() const;
	T dot(const Unit2D<T>& u) const;
	double angle(const Unit2D<T>& a, const Unit2D<T>& b) const;
	double slope(const Unit2D<T>& v) const;
	int posAtLine(const Unit2D<T>& p1, const Unit2D<T>& p2) const;
	void unitXY();

	Unit2D<T> operator+(const Unit2D<T>& u) const;
	Unit2D<T> operator-(const Unit2D<T>& u) const;
	Unit2D<T>& operator+=(const Unit2D<T>& u);
	Unit2D<T>& operator-=(const Unit2D<T>& u);
	Unit2D<T> operator*(const Unit2D<T> u) const;
	Unit2D<T> operator*(T u) const;
	Unit2D<T>& operator*=(T u);
	Unit2D<T> operator/(const Unit2D<T> u) const;
	Unit2D<T> operator/(T u) const;
	Unit2D<T>& operator/=(T u);
	bool operator==(const Unit2D<T>& u) const;
	bool operator!=(const Unit2D<T>& u) const;
	friend ostream& operator<<(ostream& os, Unit2D<T> u)
	{
		os << u.x << ", " << u.y << endl;
		return os;
	}

};

typedef Unit2D<int> Unit2Di;
typedef Unit2D<unsigned int> Unit2Du;
typedef Unit2D<float> Unit2Df;
typedef Unit2D<double> Unit2Dd;

template<class T>
inline Unit2D<T> Unit2D<T>::create(T v)
{
	return { v, v };
}

template<class T>
inline Unit2D<T> Unit2D<T>::create(T x, T y)
{
	return { x, y };
}

template<class T>
double Unit2D<T>::length() const
{
	return sqrt(x * x + y * y);
}

template<class T>
inline T Unit2D<T>::dot(const Unit2D<T>& u) const
{
	return x * u.x + y* u.y;
}

template<class T>
double Unit2D<T>::angle(const Unit2D<T>& a, const Unit2D<T>& b) const
{
	return acosf(a * b / (a.length() * b.length()));
}

template<class T>
double Unit2D<T>::slope(const Unit2D<T>& v) const
{
	return (y - v.y) / (double)(x - v.x);
}

template<class T>
int Unit2D<T>::posAtLine(const Unit2D<T>& p1, const Unit2D<T>& p2) const
{
	int a = p1.slope(p2);
	return y - a * x - p1.y + a * p1.x;
}

template<class T>
void Unit2D<T>::unitXY()
{
	x = ((x == 0) ? 0 : ((x > 0) * 2 - 1));
	y = ((y == 0) ? 0 : ((y > 0) * 2 - 1));
}

template<class T>
Unit2D<T> Unit2D<T>::operator+(const Unit2D<T>& u) const
{
	return{ x + u.x, y + u.y };
}

template<class T>
Unit2D<T> Unit2D<T>::operator-(const Unit2D<T>& u) const
{
	return{ x - u.x, y - u.y };
}

template<class T>
Unit2D<T>& Unit2D<T>::operator+=(const Unit2D<T>& u)
{
	x += u.x;
	y += u.y;
	return *this;
}

template<class T>
Unit2D<T>& Unit2D<T>::operator-=(const Unit2D<T>& u)
{
	x -= u.x;
	y -= u.y;
	return *this;
}

template<class T>
inline Unit2D<T> Unit2D<T>::operator*(const Unit2D<T> u) const
{
	return { x * u.x, y * u.y };
}

template<class T>
inline Unit2D<T> Unit2D<T>::operator*(T u) const
{
	return { x * u, y * u };
}

template<class T>
inline Unit2D<T>& Unit2D<T>::operator*=(T u)
{
	x *= u;
	y *= u;
	return *this;
}

template<class T>
inline Unit2D<T> Unit2D<T>::operator/(const Unit2D<T> u) const
{
	return { x / u.x, y / u.y };
}

template<class T>
inline Unit2D<T> Unit2D<T>::operator/(T u) const
{
	return { x / u, y / u };
}

template<class T>
inline Unit2D<T>& Unit2D<T>::operator/=(T u)
{
	x /= u;
	y /= u;
	return *this;
}

template<class T>
bool Unit2D<T>::operator==(const Unit2D<T>& u) const
{
	return x == u.x && y == u.y;
}

template<class T>
bool Unit2D<T>::operator!=(const Unit2D<T>& u) const
{
	return x != u.x || y != u.y;
}

struct ENGINE_API Color
{
	float r = 0, g = 0, b = 0, a = 0;

	Color() {}
	Color(const Vector4f& v) : r(v.x()), g(v.y()), b(v.z()), a(v.w()) {}
	Color(const Vector3f& v) : r(v.x()), g(v.y()), b(v.z()), a(1.0f) {}
	Color(int r, int g, int b, int a = 255) : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a / 255.0f) {}
	Color(float r, float g, float b, float a = 1) : r(r), g(g), b(b), a(a) {}

	Color& fromHSV(const Vector3f& hsv);
	Vector3f toHSV() const;

	static Color HSV(const Vector3f& hsv);

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
};

struct LineDrawData
{
	Vector4f p0;
	Color c0;
	Vector4f p1;
	Color c1;
};

enum TransformSpace {
	WORLD, LOCAL, RELATE
};

struct DrawArraysIndirectCommand {
	unsigned int count;
	unsigned int instanceCount;
	unsigned int first;
	unsigned int baseInstance;
};

struct DrawElementsIndirectCommand {
	unsigned int count;
	unsigned int instanceCount;
	unsigned int firstIndex;
	unsigned int baseVertex;
	unsigned int baseInstance;
};

struct ENGINE_API InstanceDrawData
{
	unsigned int instanceID;
	unsigned int baseVertex;

	InstanceDrawData& operator++();
};

template<class T>
struct MatAttribute
{
	T val;
	unsigned int idx = -1;

	MatAttribute();
	MatAttribute(const T& t);
};

template<class T>
inline MatAttribute<T>::MatAttribute()
{
}

template<class T>
inline MatAttribute<T>::MatAttribute(const T& t)
{
	val = t;
}

template<class T>
class Sampler
{
public:
	Sampler(size_t size = 1);

	void setMaxSize(size_t size);
	size_t getMaxSize() const;
	size_t getSize() const;

	void add(const T& s);
	T get(size_t index);
	void clear();

	template<class F = T>
	F getAverage() const;
	T getMax() const;
	T getMin() const;
	const list<T>& getSamples() const;

	T* toArray(bool atStack = true);
protected:
	list<T> sample;
	size_t size;
};

template<class T>
inline Sampler<T>::Sampler(size_t size)
{
	setMaxSize(size);
}

template<class T>
inline void Sampler<T>::setMaxSize(size_t size)
{
	this->size = size >= 1 ? size : 1;
	if (sample.size() > size)
		sample.resize(size);
}

template<class T>
inline size_t Sampler<T>::getMaxSize() const
{
	return size;
}

template<class T>
inline size_t Sampler<T>::getSize() const
{
	return sample.size();
}

template<class T>
inline void Sampler<T>::add(const T& s)
{
	sample.push_front(s);
	while (sample.size() > size)
		sample.pop_back();
}

template<class T>
inline T Sampler<T>::get(size_t index)
{
	if (index >= sample.size())
		return T();
	size_t i = 0;
	auto b = sample.begin();
	for (int i = 0; i < index; i++)
		++b;
	return *b;
}

template<class T>
inline void Sampler<T>::clear()
{
	sample.clear();
}

template<class T>
template<class F>
inline F Sampler<T>::getAverage() const
{
	F sum = F();
	for (auto b = sample.begin(), e = sample.end(); b != e; b++) {
		sum += *b;
	}
	return sum * (1.0 / sample.size());
}

template<class T>
inline T Sampler<T>::getMax() const
{
	if (sample.empty())
		return T();
	T m = sample.front();
	for (auto b = sample.begin(), e = sample.end(); b != e; b++)
		if (m < *b)
			m = *b;
	return m;
}

template<class T>
inline T Sampler<T>::getMin() const
{
	if (sample.empty())
		return T();
	T m = sample.front();
	for (auto b = sample.begin(), e = sample.end(); b != e; b++)
		if (m > *b)
			m = *b;
	return m;
}

template<class T>
inline const list<T>& Sampler<T>::getSamples() const
{
	return sample;
}

template<class T>
inline T* Sampler<T>::toArray(bool atStack)
{
	T* array = NULL;
	if (atStack)
		array = (T*)alloca(sizeof(T) * sample.size());
	else
		array = (T*)malloc(sizeof(T) * sample.size());
	int i = 0;
	for (auto b = sample.begin(), e = sample.end(); b != e; b++, i++)
		array[i] = *b;
	return array;
}

struct ENGINE_API Time : public chrono::steady_clock::time_point
{
public:
	Time(const __int64& nanosecond = 0);
	Time(const chrono::steady_clock::time_point& t);
	Time(const chrono::duration<long long, nano>& t);
	Time(const Time& t);

	inline unsigned long long toNanosecond() const {
		return time_since_epoch().count();
	}

	inline double toMillisecond() const {
		return time_since_epoch().count() * 0.000001;
	}

	inline double toSecond() const {
		return time_since_epoch().count() * 0.000000001;
	}

	unsigned int getDay() const;
	unsigned int getHour() const;
	unsigned int getMinute() const;
	unsigned int getSecond() const;
	unsigned int getMillisecond() const;
	unsigned int getMicrosecond() const;
	unsigned int getNanosecond() const;

	string toString() const;

	operator __int64() const;

	static Time now();
	static Time duration();
	static Time delta();
	static Time frameTime();
	static unsigned long long frames();

	static void update();

protected:
	static Time startTime;
	static Time lastTime;
	static Time deltaTime;
	static atomic_ullong frameCount;
};

class ENGINE_API Timer
{
public:
	Time initTime;
	bool isInterval = false;
	vector<pair<string, Time>> times;

	Timer();
	Timer(bool isInterval);
	Timer(const Timer& timer);

	void setIntervalMode(bool interval);
	void record(const string& label);
	void record(const string& label, const Time& interval);
	Timer& reset();
	Time getInterval(int index);
	Time getDuration();
	string toString();
};

struct ENGINE_API Particle
{
	Vector3f position;
	float lifetime;
	Vector3f scale;
	float maxLifetime;
	Vector3f velocity;
	float type;
	Vector3f acceleration;
	float extSca;
	Vector4f extVec;
	Color color;

	Particle();
	Particle(const Particle& p);

	void update(float deltaTime);
};

template<class T>
struct Range
{
	T minVal, maxVal;
	unsigned int seed = 0;

	Range();
	Range(const T& minVal, const T& maxVal);
	Range(const T& v);

	T diff();
	T mid();
	T uniform();
};

template<class T>
inline Range<T>::Range()
{
}

template<class T>
inline Range<T>::Range(const T& minVal, const T& maxVal)
{
	seed = time(NULL);
	this->minVal = minVal;
	this->maxVal = maxVal;
}

template<class T>
inline Range<T>::Range(const T& v)
{
	seed = time(NULL);
	minVal = maxVal = v;
}

template<class T>
inline T Range<T>::diff()
{
	return maxVal - minVal;
}

template<class T>
inline T Range<T>::mid()
{
	return (maxVal + minVal) * 0.5;
}

template<class T>
inline T Range<T>::uniform()
{
	if (minVal == maxVal)
		return minVal;
	float t;
	seed = 2045 * seed + 1;
	seed = seed % 1048576;
	t = seed / 1048576.0;
	return minVal + (maxVal - minVal) * t;
}

template<>
inline Vector3f Range<Vector3f>::uniform()
{
	if (minVal == maxVal)
		return minVal;
	Vector3f v = minVal;
	float t;
	seed = 2045 * seed + 1;
	seed = seed % 1048576;
	t = seed / 1048576.0;
	v.x() += (maxVal.x() - minVal.x()) * t;
	seed = 2045 * seed + 1;
	seed = seed % 1048576;
	t = seed / 1048576.0;
	v.y() += (maxVal.y() - minVal.y()) * t;
	seed = 2045 * seed + 1;
	seed = seed % 1048576;
	t = seed / 1048576.0;
	v.z() += (maxVal.z() - minVal.z()) * t;
	return v;
}

template<>
inline Color Range<Color>::uniform()
{
	if (minVal == maxVal)
		return minVal;
	Color v = minVal;
	float t;
	seed = 2045 * seed + 1;
	seed = seed - (seed / 1048576);
	t = seed / 1048576.0;
	v.r += (maxVal.r - minVal.r) * t;
	seed = 2045 * seed + 1;
	seed = seed - (seed / 1048576);
	t = seed / 1048576.0;
	v.g += (maxVal.g - minVal.g) * t;
	seed = 2045 * seed + 1;
	seed = seed - (seed / 1048576);
	t = seed / 1048576.0;
	v.b += (maxVal.b - minVal.b) * t;
	seed = 2045 * seed + 1;
	seed = seed - (seed / 1048576);
	t = seed / 1048576.0;
	v.a += (maxVal.a - minVal.a) * t;
	return v;
}

#endif // !_UNIT_H_
