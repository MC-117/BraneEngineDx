#pragma once
#include "MidiDevice.h"
#include "../Core/Base.h"
#include "../Core/Audio/AudioSource.h"

class MidiInstrument : public Base
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

	void bindMidiState(MidiState& state);
	void unbindMidiState();

	AudioSource* getNoteAudioSource(uint8_t note);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	struct NoteInfo
	{
		Zone* zone;
		float offsetPitch;
		float offsetGain;
		AudioSource audioSource;
	};
	MidiState* midiState = NULL;
	map<uint8_t, NoteInfo> noteAudioSources;
	map<uint8_t, NoteInfo*> activeNotes;

	DelegateHandle onNoteOnHandle;
	DelegateHandle onNoteOffHandle;
	DelegateHandle onPitchBendHandle;
	DelegateHandle onChannelPressureHandle;

	void onNoteOn(const MidiNoteState& state);
	void onNoteOff(const MidiNoteState& state);
	void onChannelPressure(uint8_t channel, uint8_t pressure);
};
