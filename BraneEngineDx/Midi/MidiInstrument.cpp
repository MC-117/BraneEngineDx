#include "MidiInstrument.h"
#include "../Core/Utility/MathUtility.h"
#include "../Core/Asset.h"
#include "../Core/Console.h"

SerializeInstance(MidiInstrument);

MidiInstrument::MidiInstrument(const string& name) : name(name)
{
}

MidiInstrument::~MidiInstrument()
{
	unbindMidiState();
	resetAudioSources();
}

MidiInstrument::Zone* MidiInstrument::addAudioData(AudioData& audioData)
{
	if (audioData.wave.loops.empty())
		return NULL;
	Zone& zone = zones.emplace_back();
	zone.rootNote		= audioData.wave.instrument.rootNote;
	zone.fineTune		= audioData.wave.instrument.fineTune;
	zone.gain			= audioData.wave.instrument.gain;
	zone.lowNote		= audioData.wave.instrument.lowNote;
	zone.highNote		= audioData.wave.instrument.highNote;
	zone.lowVelocity	= audioData.wave.instrument.lowVelocity;
	zone.highVelocity	= audioData.wave.instrument.highVelocity;
	zone.loopStart		= audioData.wave.loops.front().start;
	zone.loopEnd		= audioData.wave.loops.front().end;
	zone.audioData		= &audioData;
	return &zone;
}

void MidiInstrument::build()
{
	resetAudioSources();
	noteAudioSources.clear();
	for (auto& zone : zones) {
		for (int n = zone.lowNote; n <= zone.highNote; n++) {
			NoteInfo& noteInfo = noteAudioSources.insert(make_pair(n, NoteInfo())).first->second;
			noteInfo.note = n;
			noteInfo.zone = &zone;
			noteInfo.offsetPitch = 1 + (n - zone.rootNote + zone.fineTune / 100.0f) * (1.0f / 12.0f);
			noteInfo.offsetGain = 1 + zone.gain / 6;
		}
	}
}

void MidiInstrument::bindMidiState(MidiState& state)
{
	if (midiState == &state)
		return;
	unbindMidiState();
	midiState = &state;
	onNoteOnHandle = state.onNoteOnDelegate.add(*this, &MidiInstrument::onNoteOn, std::placeholders::_1);
	onNoteOffHandle = state.onNoteOffDelegate.add(*this, &MidiInstrument::onNoteOff, std::placeholders::_1);
	onChannelPressureHandle = state.onChannelPressureDelegate.add(*this, &MidiInstrument::onChannelPressure, std::placeholders::_1, std::placeholders::_2);
}

void MidiInstrument::unbindMidiState()
{
	if (midiState == NULL)
		return;
	midiState->onNoteOnDelegate -= onNoteOnHandle;
	midiState->onNoteOffDelegate -= onNoteOffHandle;
	midiState->onChannelPressureDelegate -= onChannelPressureHandle;
	midiState->onChannelPressureDelegate -= onChannelPressureHandle;
	midiState = NULL;
}

void MidiInstrument::resetAudioSources()
{
	for (auto& item : noteAudioSources) {
		item.second.audioSource = NULL;
	}
	for (auto source : pendingSourceList) {
		delete source;
	}
	pendingSourceList.clear();
	while (!idleSourceQueue.empty()) {
		delete idleSourceQueue.front();
		idleSourceQueue.pop();
	}
}

Serializable* MidiInstrument::instantiate(const SerializationInfo& from)
{
	return new MidiInstrument(from.name);
}

bool MidiInstrument::deserialize(const SerializationInfo& from)
{
	if (!Base::deserialize(from))
		return false;
	const SerializationInfo* zoneInfos = from.get("zones");
	if (zoneInfos) {
		zones.resize(zoneInfos->sublists.size());
		int i = 0;
		for (auto& item : zoneInfos->sublists) {
			Zone& zone = zones[i];
			int value;
			if (item.get("rootNote", value))
				zone.rootNote = value;
			if (item.get("fineTune", value))
				zone.fineTune = value;
			if (item.get("gain", value))
				zone.gain = value;
			if (item.get("lowNote", value))
				zone.lowNote = value;
			if (item.get("highNote", value))
				zone.highNote = value;
			if (item.get("lowVelocity", value))
				zone.lowVelocity = value;
			if (item.get("highVelocity", value))
				zone.highVelocity = value;
			if (item.get("loopStart", value))
				zone.loopStart = value;
			if (item.get("loopEnd", value))
				zone.loopEnd = value;
			string path;
			if (item.get("audioData", path))
				zone.audioData = getAssetByPath<AudioData>(path);
			else
				zone.audioData = NULL;
			i++;
		}
	}
	return true;
}

bool MidiInstrument::serialize(SerializationInfo& to)
{
	if (!Base::serialize(to))
		return false;
	SerializationInfo* zoneInfos = to.add("zones");
	if (zoneInfos) {
		int i = 0;
		for (Zone& zone : zones) {
			SerializationInfo* item = zoneInfos->push();
			int value;
			item->set("rootNote", (int)zone.rootNote);
			item->set("fineTune", (int)zone.fineTune);
			item->set("gain", (int)zone.gain);
			item->set("lowNote", (int)zone.lowNote);
			item->set("highNote", (int)zone.highNote);
			item->set("lowVelocity", (int)zone.lowVelocity);
			item->set("highVelocity", (int)zone.highVelocity);
			item->set("loopStart", (int)zone.loopStart);
			item->set("loopEnd", (int)zone.loopEnd);
			string path = AssetInfo::getPath(zone.audioData);
			item->set("audioData", path);
		}
	}
	return true;
}

AudioSource* MidiInstrument::activeNote(NoteInfo& noteInfo)
{
	AudioSource* source = NULL;
	if (idleSourceQueue.empty())
		source = new AudioSource();
	else {
		source = idleSourceQueue.front();
		idleSourceQueue.pop();
	}
	noteInfo.audioSource = source;
	source->setAudioData(noteInfo.zone->audioData);
	return source;
}

void MidiInstrument::deactiveNote(NoteInfo& noteInfo)
{
	if (noteInfo.audioSource == NULL)
		return;
	noteInfo.audioSource->setLoop(false);
	pendingSourceList.push_back(noteInfo.audioSource);
	noteInfo.audioSource = NULL;
}

void MidiInstrument::processPendingAudioSources()
{
	for (auto b = pendingSourceList.begin(); b != pendingSourceList.end();) {
		if ((*b)->getState() != AudioSource::Playing) {
			idleSourceQueue.push(*b);
			b = pendingSourceList.erase(b);
		}
		else b++;
	}
}

void MidiInstrument::onNoteOn(const MidiNoteState& state)
{
	auto iter = noteAudioSources.find(state.note);
	if (iter == noteAudioSources.end())
		return;
	processPendingAudioSources();
	NoteInfo& noteInfo = iter->second;
	AudioSource* source = activeNote(noteInfo);
	Zone* zone = iter->second.zone;
	MidiChannelState& channelState = midiState->channelStates[state.channel];
	
	float velocityScale = clamp(state.velocity, zone->lowVelocity, zone->highVelocity) / (float)0x7f;
	float pitchBend = ((channelState.pitchBend / (float)0x4000) * 2 - 1)* (1.0f / 6.0f);

	source->setPitch(noteInfo.offsetPitch + pitchBend);
	source->setVolume(noteInfo.offsetGain * velocityScale);
	source->play();
}

void MidiInstrument::onNoteOff(const MidiNoteState& state)
{
	auto iter = noteAudioSources.find(state.note);
	if (iter == noteAudioSources.end())
		return;
	NoteInfo& noteInfo = iter->second;
	deactiveNote(noteInfo);
}

void MidiInstrument::onChannelPressure(uint8_t channel, uint8_t pressure)
{
}
