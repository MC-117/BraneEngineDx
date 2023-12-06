#pragma once
#include "../Core/GUI/UIWindow.h"
#include "MidiInstrumentEditor.h"

class ENGINE_API MidiInstrumentWindow : public UIWindow
{
public:
	MidiInstrumentWindow(string name = "MidiInstrumentWindow", bool defaultShow = false);

	void setMidiInstrument(MidiInstrument* instrument);

	virtual void onWindowGUI(GUIRenderInfo& info);
protected:
	Ref<MidiInstrument> instrument;
	string newInstrumentName = "Unnamed";
};