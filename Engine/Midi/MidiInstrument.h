#pragma once
#include "MidiDevice.h"
#include "../Core/Base.h"
#include "../Core/Audio/AudioSource.h"

class ENGINE_API MidiInstrument : public Base, public IMidiStateReceivable
{
public:
	Serialize(MidiInstrument, Base);

	struct Zone
	{
		uint8_t rootNote;
		int8_t fineTune;
		int8_t gain;
		uint8_t lowNote;
		uint8_t highNote;
		uint8_t lowVelocity;
		uint8_t highVelocity;
		int32_t loopStart;
		int32_t loopEnd;
		AudioData* audioData;
	};

	string name;

	vector<Zone> zones;
	
	MidiInstrument(const string& name = "Unnamed");
	virtual ~MidiInstrument();

	Zone* addAudioData(AudioData& audioData);

	void build();

	virtual void bindMidiState(MidiState& state);
	virtual void unbindMidiState();

	void resetAudioSources();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	struct NoteInfo
	{
		Zone* zone = NULL;
		uint8_t note = 0;
		float offsetPitch = 0;
		float offsetGain = 0;
		AudioSource* audioSource = NULL;
	};
	MidiState* midiState = NULL;
	map<uint8_t, NoteInfo> noteAudioSources;

	queue<AudioSource*> idleSourceQueue;
	list<AudioSource*> pendingSourceList;

	DelegateHandle onNoteOnHandle;
	DelegateHandle onNoteOffHandle;
	DelegateHandle onPitchBendHandle;
	DelegateHandle onChannelPressureHandle;

	AudioSource* activeNote(NoteInfo& noteInfo);
	void deactiveNote(NoteInfo& noteInfo);
	void processPendingAudioSources();

	void onNoteOn(const MidiNoteState& state);
	void onNoteOff(const MidiNoteState& state);
	void onChannelPressure(uint8_t channel, uint8_t pressure);
};
