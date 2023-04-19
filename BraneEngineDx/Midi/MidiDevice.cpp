#include "MidiDevice.h"

MidiChannelState::MidiChannelState()
{
    reset();
}

void MidiChannelState::reset()
{
    memset(this, 0, sizeof(MidiChannelState));
}

void MidiState::onFetchMessage(const MidiMessage& msg)
{
    MidiMessageType msgType = msg.get_message_type();
    int channel = msg.get_channel();
    MidiChannelState& channelState = channelStates[channel];
    switch (msgType)
    {
    default:
        onFetchMessageDelegate(msgType, msg);
    case MidiMessageType::NOTE_ON:
    case MidiMessageType::NOTE_OFF:
    case MidiMessageType::POLY_PRESSURE:
    {
        MidiNoteState& noteState = channelState.noteStates[msg[1]];
        noteState.note = msg[1];
        if (msgType == MidiMessageType::POLY_PRESSURE) {
            noteState.pressure = msg[2];
            onNotePressureChangeDelegate(noteState);
        }
        else {
            noteState.isOn = msgType == MidiMessageType::NOTE_ON;
            noteState.velocity = msg[2];
            if (noteState.isOn)
                onNoteOnDelegate(noteState);
            else
                onNoteOffDelegate(noteState);
        }
        break;
    }
    case MidiMessageType::CONTROL_CHANGE:
    {
        MidiControlState& controlState = channelState.controlStates[msg[1]];
        controlState.controlID = msg[1];
        controlState.value = msg[2];
        onControlChangeDelegate(controlState);
        break;
    }
    case MidiMessageType::PITCH_BEND:
        channelState.pitchBend = msg[2] << 7 | msg[1];
        onPitchBendDelegate(channel, channelState.pitchBend);
        break;
    case MidiMessageType::AFTERTOUCH:
        channelState.pressure = msg[1];
        onChannelPressureDelegate(channel, channelState.pressure);
        break;
    }
}

void MidiState::clearDelegates()
{
    onFetchMessageDelegate.clear();
    onNoteOnDelegate.clear();
    onNoteOffDelegate.clear();
    onNotePressureChangeDelegate.clear();
    onControlChangeDelegate.clear();
    onPitchBendDelegate.clear();
    onChannelPressureDelegate.clear();
}

MidiDevice::~MidiDevice()
{
    closePort();
}

bool MidiDevice::isValid() const
{
	return midiIn.is_port_open();
}

int MidiDevice::getPort() const
{
    return port;
}

const string& MidiDevice::getName() const
{
    return name;
}

void MidiDevice::enumPorts(vector<string>& ports)
{
    int portCount = midiIn.get_port_count();
    ports.resize(portCount);
    for (int i = 0; i < portCount; i++)
    {
        ports[i] = midiIn.get_port_name(i);
    }
}

bool MidiDevice::openPort(int port)
{
    closePort();
    midiIn.open_port(port);
    midiIn.set_callback([&](const libremidi::message& message)
        {
            state.onFetchMessage(message);
        });
    if (midiIn.is_port_open()) {
        this->port = port;
        name = midiIn.get_port_name(port);
        return true;
    }
	return false;
}

void MidiDevice::closePort()
{
    midiIn.close_port();
    midiIn.cancel_callback();
    port = -1;
    name.clear();
}

MidiDevice& MidiDevice::defaultDevice()
{
    static MidiDevice device;
    return device;
}
