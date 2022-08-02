#pragma once
#ifndef _CURVE_H_
#define _CURVE_H_

#include "../Unit.h"

template<class ValueT, class BaseT = float>
struct CurveValue
{
	enum KeySmoothMode
	{
		Linear, Step, Auto, Custom
	} mode = Auto;
	ValueT value;
	ValueT inTangent;
	ValueT inRate;
	ValueT outTangent;
	ValueT outRate;

	CurveValue(KeySmoothMode mode, const ValueT& value);
	CurveValue(const ValueT& value, const ValueT& inTangent, const ValueT& outTangent, const ValueT& inRate = ValueT(), const ValueT& outRate = ValueT());

	ValueT lerp(CurveValue<ValueT, BaseT> target, BaseT rate);
	void caculateTangent();

	static BaseT biBezier(BaseT v0, BaseT r0, BaseT v1, BaseT r1, BaseT v2, BaseT r2, BaseT rate);
	static BaseT cuBezier(BaseT v0, BaseT r0, BaseT v1, BaseT r1, BaseT v2, BaseT r2, BaseT v3, BaseT r3, BaseT rate);
};

template<>
struct CurveValue<Quaternionf>
{
	enum KeySmoothMode
	{
		Linear, Step, Auto, Custom
	} mode = Auto;
	Quaternionf value;
	float inTangent;
	float inRate = 0;
	float outTangent;
	float outRate = 0;

	CurveValue(KeySmoothMode mode, const Quaternionf& value);
	CurveValue(const Quaternionf& value, float inTangent, float outTangent, float inRate = 0, float outRate = 0);

	Quaternionf lerp(CurveValue<Quaternionf> target, float rate);
	void caculateTangent();

	static float biBezier(float v0, float r0, float v1, float r1, float v2, float r2, float rate);
	static float cuBezier(float v0, float r0, float v1, float r1, float v2, float r2, float v3, float r3, float rate);
};

template<class KetT, class ValueT>
class Curve
{
public:
	map<KetT, CurveValue<ValueT>> keys;
	KetT duration;
	bool empty() const;
	void insert(const KetT& key, const CurveValue<ValueT>& value);
	ValueT get(const KetT& key);
};

template<class KetT, class ValueT>
class CurvePlayer
{
public:
	Curve<KetT, ValueT>* curve = NULL;
	KetT keyCount = KetT();
	bool loop = false;
	bool isPlaying = false;
	float speed = 1;
	typename map<KetT, CurveValue<ValueT>>::iterator curveIterator;
	
	CurvePlayer();
	CurvePlayer(Curve<KetT, ValueT>& curve);

	void setCurve(Curve<KetT, ValueT>& curve);
	ValueT update(const KetT& deltaKey);
	ValueT setTime(const KetT& time);
	void play();
	void pause();
	void stop();
	bool playing();
	void setSpeed(float speed);
	void reset();
};

template<class ValueT, class BaseT>
inline CurveValue<ValueT, BaseT>::CurveValue(KeySmoothMode mode, const ValueT & value)
	: mode(mode), value(value)
{
	caculateTangent();
}

template<class ValueT, class BaseT>
inline CurveValue<ValueT, BaseT>::CurveValue(const ValueT & value, const ValueT & inTangent, const ValueT & outTangent, const ValueT & inRate, const ValueT & outRate)
	: mode(Custom), value(value), inTangent(inTangent), outTangent(outTangent), inRate(inRate), outRate(outRate)
{
}

template<class ValueT, class BaseT>
inline ValueT CurveValue<ValueT, BaseT>::lerp(CurveValue<ValueT, BaseT> target, BaseT r)
{
	r = max(min(r, 1), 0);
	BaseT r1 = 1 - r;
	const unsigned int baseSize = sizeof(ValueT) / sizeof(BaseT);
	if (mode == Step)
		return value;
	else if (mode == target.mode) {
		if (mode == Linear)
			return value * r1 + target.value * r;
	}
	else if (mode == Linear) {
		ValueT v;
		for (int i = 0; i < baseSize; i++) {
			BaseT _r = biBezier(BaseT(0), BaseT(0), ((BaseT*)&target.inTangent)[i], ((BaseT*)&target.inRate)[i], BaseT(1), BaseT(1), r);
			((BaseT*)&v)[i] = ((BaseT*)&value)[i] * (1 - _r) + ((BaseT*)&target.value)[i] * _r;
		}
		return v;
	}
	else if (target.mode == Linear) {
		ValueT v;
		for (int i = 0; i < baseSize; i++) {
			BaseT _r = biBezier(BaseT(0), BaseT(0), ((BaseT*)&outTangent)[i], ((BaseT*)&outRate)[i], BaseT(1), BaseT(1), r);
			((BaseT*)&v)[i] = ((BaseT*)&value)[i] * (1 - _r) + ((BaseT*)&target.value)[i] * _r;
		}
		return v;
	}
	ValueT v;
	for (int i = 0; i < baseSize; i++) {
		BaseT _r = cuBezier(BaseT(0), BaseT(0), ((BaseT*)&outTangent)[i], ((BaseT*)&outRate)[i], ((BaseT*)&target.inTangent)[i], ((BaseT*)&target.inRate)[i], BaseT(1), BaseT(1), r);
		((BaseT*)&v)[i] = ((BaseT*)&value)[i] * (1 - _r) + ((BaseT*)&target.value)[i] * _r;
	}
	return v;
}

template<class ValueT, class BaseT>
inline void CurveValue<ValueT, BaseT>::caculateTangent()
{
	/*switch (mode)
	{
	case Linear:
	case Step:
		inTangent = ValueT();
		outTangent = ValueT();
		break;
	case Auto:
		inTangent = ValueT(value);
		outTangent = ValueT(value);
	default:
		break;
	}*/
}

template<class ValueT, class BaseT>
inline BaseT CurveValue<ValueT, BaseT>::biBezier(BaseT v0, BaseT r0, BaseT v1, BaseT r1, BaseT v2, BaseT r2, BaseT rate)
{
	BaseT t = 0.5;
	BaseT tprecious = 0.5;
	BaseT slope = ((v2 - v0) / (r2 - r0)) < 0 ? -1 : 1;
	while (tprecious > 0.00001) {
		BaseT t1 = 1 - t;
		BaseT b = t1 * t1 * r0 + 2 * t * t1 + t * t * r2;
		tprecious *= 0.5f;
		if (rate < b)
			t -= tprecious * slope;
		else if (b < rate)
			t += tprecious * slope;
		else break;
	}
	BaseT t1 = 1 - t;
	return t1 * t1 * v0 + 2 * t * t1 * v1 + t * t * v2;
}

template<class ValueT, class BaseT>
inline BaseT CurveValue<ValueT, BaseT>::cuBezier(BaseT v0, BaseT r0, BaseT v1, BaseT r1, BaseT v2, BaseT r2, BaseT v3, BaseT r3, BaseT rate)
{
	BaseT t = 0.5;
	BaseT tprecious = 0.5;
	BaseT slope = ((v3 - v0) / (r3 - r0)) < 0 ? -1 : 1;
	while (tprecious > 0.00001) {
		BaseT t1 = 1 - t;
		BaseT b = t1 * t1 * t1 * r0 + 3 * t * t1 * t1 * r1 + 3 * t * t * t1 * r2 + t * t * t * r3;
		tprecious *= 0.5f;
		if (rate < b)
			t -= tprecious * slope;
		else if (b < rate)
			t += tprecious * slope;
		else break;
	}
	BaseT t1 = 1 - t;
	return t1 * t1 * t1 * v0 + 3 * t * t1 * t1 * v1 + 3 * t * t * t1 * v2 + t * t * t * v3;
}

inline CurveValue<Quaternionf>::CurveValue(KeySmoothMode mode, const Quaternionf & value)
	: mode(mode), value(value)
{
	caculateTangent();
}

inline CurveValue<Quaternionf>::CurveValue(const Quaternionf & value, float inTangent, float outTangent, float inRate, float outRate)
	: mode(Custom), value(value), inTangent(inTangent), outTangent(outTangent), inRate(inRate), outRate(outRate)
{
}

inline Quaternionf CurveValue<Quaternionf>::lerp(CurveValue<Quaternionf> target, float r)
{
	r = max(min(r, 1.0f), 0.0f);
	if (mode == Step)
		return value;
	if (mode == target.mode) {
		if (mode != Linear)
			r = cuBezier(0, 0, outTangent, outRate, target.inTangent, target.inRate, 1, 1, r);
	}
	else if (mode == Linear)
		r = biBezier(0, 0, target.inTangent, target.inRate, 1, 1, r);
	else if (target.mode == Linear)
		r = biBezier(0, 0, outTangent, outRate, 1, 1, r);
	else
		r = cuBezier(0, 0, outTangent, outRate, target.inTangent, target.inRate, 1, 1, r);
	return value.slerp(r, target.value);
}

inline void CurveValue<Quaternionf>::caculateTangent()
{

}

inline float CurveValue<Quaternionf>::biBezier(float v0, float r0, float v1, float r1, float v2, float r2, float rate)
{
	float t = 0.5f;
	float tprecious = 0.5;
	float slope = ((v2 - v0) / (r2 - r0)) < 0 ? -1 : 1;
	while (tprecious > 0.00001f) {
		float t1 = 1 - t;
		float b = t1 * t1 * r0 + 2 * t * t1 + t * t * r2;
		tprecious *= 0.5f;
		if (rate < b)
			t -= tprecious * slope;
		else if (b < rate)
			t += tprecious * slope;
		else break;
	}
	float t1 = 1 - t;
	return t1 * t1 * v0 + 2 * t * t1 * v1 + t * t * v2;
}

inline float CurveValue<Quaternionf>::cuBezier(float v0, float r0, float v1, float r1, float v2, float r2, float v3, float r3, float rate)
{
	float t = 0.5f;
	float tprecious = 0.5;
	float slope = ((v3 - v0) / (r3 - r0)) < 0 ? -1 : 1;
	while (tprecious > 0.00001f) {
		float t1 = 1 - t;
		float b = t1 * t1 * t1 * r0 + 3 * t * t1 * t1 * r1 + 3 * t * t * t1 * r2 + t * t * t * r3;
		tprecious *= 0.5f;
		if (rate < b)
			t -= tprecious * slope;
		else if (b < rate)
			t += tprecious * slope;
		else break;
	}
	float t1 = 1 - t;
	return t1 * t1 * t1 * v0 + 3 * t * t1 * t1 * v1 + 3 * t * t * t1 * v2 + t * t * t * v3;
}

template<class KetT, class ValueT>
inline bool Curve<KetT, ValueT>::empty() const
{
	return keys.empty();
}

template<class KetT, class ValueT>
inline void Curve<KetT, ValueT>::insert(const KetT & key, const CurveValue<ValueT> & value)
{
	auto iter = keys.find(key);
	if (iter == keys.end())
		keys.insert(make_pair(key, value));
	else
		iter->second = value;
}

template<class KetT, class ValueT>
inline ValueT Curve<KetT, ValueT>::get(const KetT & key)
{
	if (keys.empty())
		return ValueT();
	auto re = keys.equal_range(key);
	if (re.first == keys.begin())
		return re.first->second.value;
	else if (re.first == keys.end())
		return keys.rbegin()->second.value;
	else if (re.first == re.second) {
		const KetT& kb = re.first->first;
		CurveValue<ValueT>& vb = re.first->second;
		re.first--;
		const KetT& ka = re.first->first;
		CurveValue<ValueT>& va = re.first->second;
		return va.lerp(vb, (key - ka) / (kb - ka));
	}
	else
		return re.first->second.value;
}

#endif // !_CURVE_H_

template<class KetT, class ValueT>
inline CurvePlayer<KetT, ValueT>::CurvePlayer()
{
}

template<class KetT, class ValueT>
inline CurvePlayer<KetT, ValueT>::CurvePlayer(Curve<KetT, ValueT>& curve)
{
	this->curve = &curve;
	reset();
}

template<class KetT, class ValueT>
inline void CurvePlayer<KetT, ValueT>::setCurve(Curve<KetT, ValueT>& curve)
{
	this->curve = &curve;
	reset();
}

template<class KetT, class ValueT>
inline ValueT CurvePlayer<KetT, ValueT>::update(const KetT & deltaKey)
{
	if (curve == NULL || !isPlaying)
		return ValueT();
	if (curve->empty()) {
		isPlaying = false;
		return ValueT();
	}
	keyCount += deltaKey * speed;
	if (curveIterator == curve->keys.begin()) {
		if (keyCount <= curveIterator->first)
			return curveIterator->second.value;
	}
	auto iter = curveIterator;
	while (iter != curve->keys.end()) {
		if (iter->first >= keyCount) {
			break;
		}
		else {
			curveIterator = iter;
		}
		iter++;
	}
	ValueT v;
	if (iter == curve->keys.end())
		v = curveIterator->second.value;
	else
		v = curveIterator->second.lerp(iter->second, (keyCount - curveIterator->first) /
			(iter->first - curveIterator->first));
	if (keyCount >= curve->duration / speed) {
		reset();
		if (!loop)
			isPlaying = false;
	}
	return v;
}

template<class KetT, class ValueT>
inline ValueT CurvePlayer<KetT, ValueT>::setTime(const KetT& time)
{
	keyCount = time;
	curveIterator = curve->keys.lower_bound(time);
	if (curveIterator == curve->keys.begin()) {
		if (keyCount <= curveIterator->first)
			return curveIterator->second.value;
	}
	else {
		curveIterator--;
	}
	auto iter = curveIterator;
	while (iter != curve->keys.end()) {
		if (iter->first >= keyCount) {
			break;
		}
		else {
			curveIterator = iter;
		}
		iter++;
	}
	ValueT v;
	if (iter == curve->keys.end())
		v = curveIterator->second.value;
	else
		v = curveIterator->second.lerp(iter->second, (keyCount - curveIterator->first) /
			(iter->first - curveIterator->first));
	if (keyCount >= curve->duration / speed) {
		reset();
		if (!loop)
			isPlaying = false;
	}
	return v;
}

template<class KetT, class ValueT>
inline void CurvePlayer<KetT, ValueT>::play()
{
	isPlaying = true;
}

template<class KetT, class ValueT>
inline void CurvePlayer<KetT, ValueT>::pause()
{
	isPlaying = false;
}

template<class KetT, class ValueT>
inline void CurvePlayer<KetT, ValueT>::stop()
{
	isPlaying = false;
	reset();
}

template<class KetT, class ValueT>
inline bool CurvePlayer<KetT, ValueT>::playing()
{
	return isPlaying;
}

template<class KetT, class ValueT>
inline void CurvePlayer<KetT, ValueT>::setSpeed(float speed)
{
	this->speed = speed;
}

template<class KetT, class ValueT>
inline void CurvePlayer<KetT, ValueT>::reset()
{
	if (curve == NULL)
		return;
	curveIterator = curve->keys.begin();
	keyCount = KetT();
}
