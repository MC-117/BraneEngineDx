#pragma once
#include "../Core/Editor/BaseEditor.h"
#include "MidiInstrument.h"
#include "../VST2/Vst2Plugin.h"

class MidiInstrumentEditor : public BaseEditor
{
public:
	MidiInstrumentEditor() = default;
	virtual ~MidiInstrumentEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onMidiDeviceGUI(EditorInfo& info);
	virtual void onConfigGUI(EditorInfo& info);
	virtual void onInstrumentGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	MidiInstrument* instrument = NULL;
	Vst2PluginPtr vst2Plugin;

	bool refreshedDeviceList = false;
	vector<string> deviceNameList;
	int beginOctaveNote = 36, endOctaveNote = 97;
	int keyboardOctave = 4, keyboardChannel = 0;
	uint8_t prevMouseNote = 128;

	void pianoCallback(int Msg, uint8_t Key, float Vel);
};