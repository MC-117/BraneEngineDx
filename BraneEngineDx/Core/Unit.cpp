#include "Unit.h"

Time Time::startTime = Time::now();

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

unsigned int Time::getDay()
{
	return chrono::duration_cast<chrono::hours>(time_since_epoch()).count() / 24;
}

unsigned int Time::getHour()
{
	return chrono::duration_cast<chrono::hours>(time_since_epoch()).count() % 24;
}

unsigned int Time::getMinute()
{
	return chrono::duration_cast<chrono::minutes>(time_since_epoch()).count() % 60;
}

unsigned int Time::getSecond()
{
	return chrono::duration_cast<chrono::seconds>(time_since_epoch()).count() % 60;
}

unsigned int Time::getMillisecond()
{
	return chrono::duration_cast<chrono::milliseconds>(time_since_epoch()).count() % 1000;
}

unsigned int Time::getMicrosecond()
{
	return chrono::duration_cast<chrono::microseconds>(time_since_epoch()).count() % 1000;
}

unsigned int Time::getNanosecond()
{
	return chrono::duration_cast<chrono::nanoseconds>(time_since_epoch()).count() % 1000;
}

string Time::toString()
{
	return to_string(getHour()) + ':' + to_string(getMinute()) + ':'
		+ to_string(getSecond()) + ':' + to_string(getMillisecond());
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

Color Color::operator+(const Color& c) const
{
	return Color(r + c.r, g + c.g, b + c.b, a + c.a);
}

Color Color::operator-(const Color& c) const
{
	return Color(r - c.r, g - c.g, b - c.b, a - c.a);
}

Color Color::operator*(float s) const
{
	return Color(r * s, g * s, b * s, a * s);
}

Color Color::operator/(float s) const
{
	return Color(r / s, g / s, b / s, a / s);
}

bool Color::operator==(const Color& c) const
{
	return r == c.r && g == c.g && b == c.b && a == c.a;
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
