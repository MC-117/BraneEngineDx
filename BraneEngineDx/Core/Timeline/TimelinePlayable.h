#pragma once

#include "../Base.h"

struct PlayInfo
{
	float startTime;
	float endTime;
	float currentTime;
	float normalizedTime;
	float deltaTime;
};

class TimelinePlayable;

struct ClipInfo
{
	float startTime;
	float duration;
	TimelinePlayable* playable;
};

class TimelinePlayable : public Serializable
{
public:
	Serialize(TimelinePlayable,);

	TimelinePlayable() = default;
	virtual ~TimelinePlayable() = default;

	virtual void onBeginPlay(const PlayInfo& info);
	virtual void onPlay(const PlayInfo& info);
	virtual void onEndPlay(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};