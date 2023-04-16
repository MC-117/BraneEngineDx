#pragma once
#include "MidiDevice.h"
#include "../Core/Base.h"
#include "../Core/AudioSource.h"

struct MidiSampler
{
	struct Block
	{
		AudioData* sample;
		unsigned long long sampleStart;
		unsigned long long sampleEnd;
		unsigned char rootNote;
		unsigned char lowNote;
		unsigned char highNote;
		unsigned char lowVelocity;
		unsigned char highVelocity;
	};

	vector<Block> blocks;


};

class MidiInstrument : public Base
{
public:
	Serialize(MidiInstrument, Base);

	MidiInstrument(const string& name);
	virtual ~MidiInstrument();

	MidiDevice& getDevice();
protected:
	string name;
	MidiDevice device;
};
