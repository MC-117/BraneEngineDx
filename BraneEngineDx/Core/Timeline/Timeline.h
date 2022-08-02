#pragma once

#include "TimelineTrack.h"
#include "../Base.h"

class Timeline : public Base
{
public:
	Serialize(Timeline, Base);

	string name;
	vector<TimelineTrack*> tracks;

	Timeline() = default;
	virtual ~Timeline();

	template<class T>
	T* createTrack();

	TimelineTrack* createTrack(Serialization& serialization);

	bool removeTrack(TimelineTrack* track);
	bool removeClip(TimelineClip* clip);

	void apply();

	bool isLoop() const;
	void setLoop(bool value);

	float getDuration() const;

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	bool loop;
	float duration;
	PlayInfo playInfo;
};

template<class T>
inline T* Timeline::createTrack()
{
	static_assert(std::is_base_of<TimelineTrack, T>::value,
		"Only allow class based on TimelineTrack");
	T* track = new T;
	track->index = tracks.size();
	tracks.push_back(track);
	return track;
}
