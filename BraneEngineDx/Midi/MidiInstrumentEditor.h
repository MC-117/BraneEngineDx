#pragma once
#include "../Core/Editor/BaseEditor.h"
#include "MidiInstrument.h"

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

	bool refreshedDeviceList = false;
	vector<string> deviceNameList;
};