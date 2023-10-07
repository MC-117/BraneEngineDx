#pragma once

#include "TimelineTrack.h"
#include "TimelineClip.h"

class ENGINE_API TimelineClipTrack : public TimelineTrack
{
public:
	Serialize(TimelineClipTrack, TimelineTrack);

	vector<TimelineClip*> clips;

	TimelineClipTrack(const string& name = "TimelineClipTrack");
	virtual ~TimelineClipTrack();

	virtual TimelineClip* addClip(const ClipInfo& info);
	virtual bool removeClip(int index);
	virtual void apply();

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