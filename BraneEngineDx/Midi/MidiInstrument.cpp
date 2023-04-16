#include "MidiInstrument.h"

MidiInstrument::MidiInstrument(const string& name) : name(name)
{
}

MidiInstrument::~MidiInstrument()
{
	if (device.isValid())
		device.closePort();
}

MidiDevice& MidiInstrument::getDevice()
{
	return device;
}
