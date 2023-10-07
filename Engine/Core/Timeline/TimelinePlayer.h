#pragma once

#include "../Object.h"
#include "Timeline.h"

class ENGINE_API TimelinePlayer : public Object
{
public:
	Serialize(TimelinePlayer, Object);

	TimelinePlayer(const string& name = "TimelinePlayer");
	virtual ~TimelinePlayer();

	void setTimeline(Timeline* data);
	Timeline* getTimeline() const;

	bool isPlay() const;
	bool isLoop() const;

	void setLoop(bool value);

	virtual void tick(float deltaTime);

	float getTime() const;
	float getNormalizedTime() const;
	void setTime(float time);
	void setNormalizedTime(float time);
	void play();
	void pause();
	void stop();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	bool playing;
	bool loop;
	PlayInfo playInfo;
	Ref<Timeline> timeline;
};