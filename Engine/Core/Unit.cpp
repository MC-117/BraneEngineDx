#include "Unit.h"

Time Time::startTime = Time::now();
Time Time::lastTime;
Time Time::deltaTime;
atomic_ullong Time::frameCount = 0;

Time::Time(const __int64& nanosecond)
	: chrono::steady_clock::time_point(chrono::nanoseconds(nanosecond))
{
}

Time::Time(const chrono::steady_clock::time_point& t)
	: chrono::steady_clock::time_point(t.time_since_epoch())
{
}

Time::Time(const chrono::duration<long long, nano>& t)
	: chrono::steady_clock::time_point(t)
{
}

Time::Time(const Time& t)
	: chrono::steady_clock::time_point(t.time_since_epoch())
{
}

unsigned int Time::getDay() const
{
	return chrono::duration_cast<chrono::hours>(time_since_epoch()).count() / 24;
}

unsigned int Time::getHour() const
{
	return chrono::duration_cast<chrono::hours>(time_since_epoch()).count() % 24;
}

unsigned int Time::getMinute() const
{
	return chrono::duration_cast<chrono::minutes>(time_since_epoch()).count() % 60;
}

unsigned int Time::getSecond() const
{
	return chrono::duration_cast<chrono::seconds>(time_since_epoch()).count() % 60;
}

unsigned int Time::getMillisecond() const
{
	return chrono::duration_cast<chrono::milliseconds>(time_since_epoch()).count() % 1000;
}

unsigned int Time::getMicrosecond() const
{
	return chrono::duration_cast<chrono::microseconds>(time_since_epoch()).count() % 1000;
}

unsigned int Time::getNanosecond() const
{
	return chrono::duration_cast<chrono::nanoseconds>(time_since_epoch()).count() % 1000;
}

string Time::toString() const
{
	char buffer[11];
	sprintf_s(buffer, ":%2d:%2d:%3d", getMinute(), getSecond(), getMillisecond());
	return to_string(getHour()) + buffer;
}

Time::operator __int64() const
{
	return chrono::duration_cast<chrono::nanoseconds>(time_since_epoch()).count();
}

Time Time::now()
{
	return chrono::steady_clock::now();
}

Time Time::duration()
{
	return (now() - startTime).count();
}

Time Time::delta()
{
	return deltaTime;
}

Time Time::frameTime()
{
	return lastTime == 0 ? Time() : (lastTime - startTime);
}

unsigned long long Time::frames()
{
	return frameCount;
}

void Time::update()
{
	if (lastTime == 0) {
		lastTime = now();
		frameCount = 0;
	}
	else {
		Time cur = now();
		deltaTime = cur - lastTime;
		lastTime = cur;
		frameCount++;
	}
}

Timer::Timer()
{
	initTime = Time::now();
}

Timer::Timer(bool isInterval) : isInterval(isInterval)
{
}

Timer::Timer(const Timer& timer)
{
	initTime = timer.initTime;
	times = timer.times;
	isInterval = timer.isInterval;
}

void Timer::setIntervalMode(bool interval)
{
	isInterval = interval;
}

void Timer::record(const string& label)
{
	if (!isInterval)
		times.push_back(pair<string, Time>(label, Time::now()));
}

void Timer::record(const string& label, const Time& interval)
{
	if (isInterval)
		times.push_back(pair<string, Time>(label, interval));
}

Timer& Timer::reset()
{
	initTime = Time::now();
	times.clear();
	return *this;
}

Time Timer::getInterval(int index)
{
	if (index < 0 || index >= times.size())
		return Time();
	else if (isInterval)
		return times[index].second;
	else if (index == 0)
		return times[0].second - initTime;
	else
		return times[index].second - times[index - 1].second;
}

Time Timer::getDuration()
{
	return Time::now() - initTime;
}

string Timer::toString()
{
	stringstream str;
	str << times[0].first << ": " << Time(times[0].second - initTime).toMillisecond() << "ms;\n";
	for (int i = 1; i < times.size(); i++) {
		str << times[i].first << ": " << Time(times[i].second - times[i - 1].second).toMillisecond() << "ms;\n";
	}
	return str.rdbuf()->str();
}

Color& Color::fromHSV(const Vector3f& hsv)
{
	float h = hsv.x(), s = hsv.y(), v = hsv.z();
	float      hh, p, q, t, ff;
	long        i;
	a = 1;
	if (s <= 0.0) {       // < is bogus, just shuts up warnings
		r = v;
		g = v;
		b = v;
		return *this;
	}
	hh = h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;

	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	case 5:
	default:
		r = v;
		g = p;
		b = q;
		break;
	}
	return *this;
}

Vector3f Color::toHSV() const
{
	Vector3f         out;
	float &h = out.x(), &s = out.y(), &v = out.z();
	double      min, max, delta;

	min = r < g ? r : g;
	min = min < b ? min : b;

	max = r > g ? r : g;
	max = max > b ? max : b;

	v = max;                                // v
	delta = max - min;
	if (delta < 0.00001)
	{
		s = 0;
		h = 0; // undefined, maybe nan?
		return out;
	}
	if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
		s = (delta / max);                  // s
	}
	else {
		// if max is 0, then r = g = b = 0              
		// s = 0, h is undefined
		s = 0.0;
		h = NAN;                            // its now undefined
		return out;
	}
	if (r >= max)                           // > is bogus, just keeps compilor happy
		h = (g - b) / delta;        // between yellow & magenta
	else
		if (g >= max)
			h = 2.0 + (b - r) / delta;  // between cyan & yellow
		else
			h = 4.0 + (r - g) / delta;  // between magenta & cyan

	h *= 60.0;                              // degrees

	if (h < 0.0)
		h += 360.0;

	return out;
}

Color Color::HSV(const Vector3f& hsv)
{
	return Color().fromHSV(hsv);
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

Particle::Particle()
{
}

Particle::Particle(const Particle& p)
{
	position = p.position;
	scale = p.scale;
	velocity = p.velocity;
	acceleration = p.acceleration;
	color = p.color;
	lifetime = p.lifetime;
	maxLifetime = p.maxLifetime;
	type = p.type;
	extSca = p.extSca;
	extVec = p.extVec;
}

void Particle::update(float deltaTime)
{
	position += (velocity + acceleration * (0.5 * deltaTime)) * deltaTime;
	velocity += acceleration * deltaTime;
}

InstanceDrawData& InstanceDrawData::operator++()
{
	instanceID++;
	return *this;
}
