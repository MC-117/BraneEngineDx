#pragma once

#include <libremidi/libremidi.hpp>
#include "../Core/Config.h"
#include "../Core/Delegate.h"

typedef libremidi::message MidiMessage;
typedef libremidi::message_type MidiMessageType;

enum class MidiControlType : uint8_t
{
	BANK_SELECT = 0x00,
	MODULATION = 0x01,
	BREATH_CONTROLLER = 0x02,
	FOOT_CONTROLLER = 0x04,
	PORTAMENTO_TIME = 0x05,
	DATA_ENTRY_MSB = 0x06,
	MAIN_VOLUME = 0x07,
	BALANCE = 0x08,
	PAN = 0x0A,
	EXPRESSION = 0x0B,
	EFFECT_CONTROL_1 = 0x0C,
	EFFECT_CONTROL_2 = 0x0D,
	GENERAL_PURPOSE_CONTROLLER_1 = 0x10,
	GENERAL_PURPOSE_CONTROLLER_2 = 0x11,
	GENERAL_PURPOSE_CONTROLLER_3 = 0x12,
	GENERAL_PURPOSE_CONTROLLER_4 = 0x13,
	DAMPER_PEDAL_SUSTAIN = 0x40
};

struct MidiNoteState
{
	uint8_t channel;
	bool isOn : 1;
	uint8_t note : 7;
	uint8_t velocity;
	uint8_t pressure;
};

struct MidiControlState
{
	uint8_t channel;
	uint8_t controlID;
	uint8_t value;
	uint8_t pad;
};

struct MidiChannelState
{
	enum { MaxNotes = 128, MaxControls = 128 };
	MidiNoteState noteStates[MaxNotes];
	MidiControlState controlStates[MaxControls];
	uint16_t pitchBend;
	uint8_t pressure;

	MidiChannelState();
	void reset();
};

struct MidiState
{
	enum { MaxChannels = 16 };
	MidiChannelState channelStates[MaxChannels];

	Delegate<MidiMessageType, const MidiMessage&> onFetchMessageDelegate;
	Delegate<const MidiNoteState&> onNoteOnDelegate;
	Delegate<const MidiNoteState&> onNoteOffDelegate;
	Delegate<const MidiNoteState&> onNotePressureChangeDelegate;
	Delegate<const MidiControlState&> onControlChangeDelegate;
	Delegate<uint8_t, uint16_t> onPitchBendDelegate;
	Delegate<uint8_t, uint8_t> onChannelPressureDelegate;

	MidiState() = default;
	void onFetchMessage(const MidiMessage& msg);
	void clearDelegates();
};

class MidiDevice
{
public:
	MidiState state;

	MidiDevice() = default;

	bool isValid() const;

	void enumPorts(vector<string>& ports);
	bool openPort(int port);
	void closePort();
protected:
	libremidi::midi_in midiIn;
};