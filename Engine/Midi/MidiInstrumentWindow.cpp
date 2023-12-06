#include "MidiInstrumentWindow.h"
#include "../Core/Engine.h"
#include "../Core/GUI/GUIUtility.h"

MidiInstrumentWindow::MidiInstrumentWindow(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void MidiInstrumentWindow::setMidiInstrument(MidiInstrument* instrument)
{
	this->instrument = instrument;
}

void MidiInstrumentWindow::onWindowGUI(GUIRenderInfo& info)
{
	if (instrument == NULL) {
		ImGui::BeginHorizontal("H");
		ImGui::Spring();
		ImGui::BeginVertical("V");
		ImGui::Spring();
		ImGui::InputText("Name", &newInstrumentName);
		if (ImGui::Button("New MidiInstrument")) {
			instrument = new MidiInstrument(newInstrumentName);
		}
		ImGui::Spring();
		ImGui::EndVertical();
		ImGui::Spring();
		ImGui::EndHorizontal();
		return;
	}

	Editor* editor = EditorManager::getEditor(*instrument);
	if (editor) {
		EditorInfo eidorInfo;
		eidorInfo.destroy = false;
		eidorInfo.gui = &info.gui;
		eidorInfo.gizmo = info.gizmo;
		eidorInfo.camera = info.camera;
		eidorInfo.world = Engine::getCurrentWorld();
		editor->onGUI(eidorInfo);
	}
}
