#pragma once

#include "TimelineClip.h"

class TimelineTrack : public Base
{
public:
	Serialize(TimelineTrack, Base);

	string name;
	vector<TimelineClip*> clips;

	int index = -1;
	float startTime = 0;
	float duration = 0;

	TimelineTrack(const string& name = "TimelineTrack");
	virtual ~TimelineTrack();

	virtual TimelineClip* addClip(const ClipInfo& info);
	virtual bool removeClip(int index);
	void apply();

	virtual void onBeginPlay(const PlayInfo& info);
	virtual void onPlay(const PlayInfo& info);
	virtual void onEndPlay(const PlayInfo& info);

	virtual void onPlayBlend(const PlayInfo& info);

	virtual void onUpdate(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
	struct ActiveClip
	{
		TimelineClip* clip;
		float weight;
	};
	list<ActiveClip> activedClips;
protected:
	virtual void calculateActiveClipWeight();
};