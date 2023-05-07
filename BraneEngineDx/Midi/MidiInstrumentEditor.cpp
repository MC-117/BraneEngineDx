#include "MidiInstrumentEditor.h"
#include "../Core/Utility/EngineUtility.h"
#include "../Core/GUI/GUIUtility.h"
#include <filesystem>

RegistEditor(MidiInstrument);

void MidiInstrumentEditor::setInspectedObject(void* object)
{
	instrument = dynamic_cast<MidiInstrument*>((Base*)object);
	BaseEditor::setInspectedObject(instrument);
}

void MidiInstrumentEditor::onMidiDeviceGUI(EditorInfo& info)
{
	if (ImGui::Button("Refresh"))
		refreshedDeviceList = false;
	MidiDevice& device = MidiDevice::defaultDevice();
	if (!refreshedDeviceList) {
		refreshedDeviceList = true;
		device.enumPorts(deviceNameList);
	}
	string deviceName = device.getName();
	if (ImGui::BeginCombo("MidiDevice", device.isValid() ? (to_string(device.getPort()) + ": " + deviceName).c_str() : "(none)")) {
		for (int i = 0; i < deviceNameList.size(); i++) {
			if (ImGui::Selectable((to_string(i) + ": " + deviceNameList[i]).c_str(), i == device.getPort())) {
				device.openPort(i);
				instrument->bindMidiState(device.state);
			}
		}
		ImGui::EndCombo();
	}
	if (vst2Plugin) {
		if (ImGui::Button(("Open " + vst2Plugin->getName()).c_str())) {
			vst2Plugin->openEditor();
		}
		ImGui::SameLine();
		ImGui::Text("Time(%s) Path(%s)", vst2Plugin->getDuration().toString().c_str(), vst2Plugin->getPath().c_str());

		if (ImPlot::BeginPlot("WavPlot", { -1, 0 }, ImPlotFlags_NoLegend | ImPlotFlags_NoTitle)) {
			const vector<char>& data = vst2Plugin->getBackBuffer().wave.data;
			int channels = vst2Plugin->getBackBuffer().getChannels();
			int frameCount = data.size() / channels;
			ImPlot::SetupAxesLimits(0, frameCount, 0, 255, ImPlotCond_Always);
			ImPlotAxisFlags axisFlags = ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoLabel;
			ImPlot::SetupAxes("Frame", "Amp", axisFlags, axisFlags);
			for (int i = 0; i < channels; i++) {
				ImGui::PushID(i);
				ImPlot::PlotLine("##Channel", (const unsigned char*)data.data(), frameCount, 1.0, 0.0, ImPlotLineFlags_None, sizeof(char) * i, sizeof(char)* channels);
				ImGui::PopID();
			}
			ImPlot::EndPlot();
		}
	}
	else {
		if (ImGui::Button("Load VST2 Plugin")) {
			FileDlgDesc desc;
			desc.save = false;
			desc.initDir = "Content";
			desc.title = "Choose VST2 Plugin";
			desc.defFileExt = "dll";
			desc.filter = "dll(*.dll)|*.dll|vst(*.vst)|*.vst";
			if (openFileDlg(desc)) {
				vst2Plugin = Vst2PluginManger::instance().loadPlugin(desc.filePath);
				if (vst2Plugin) {
					vst2Plugin->bindMidiState(device.state);
				}
			}
		}
	}
}

void MidiInstrumentEditor::onConfigGUI(EditorInfo& info)
{
	if (ImGui::BeginTable("Zones", 12, ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Length");
		ImGui::TableSetupColumn("SampleRate");
		ImGui::TableSetupColumn("RootNote");
		ImGui::TableSetupColumn("FineTune");
		ImGui::TableSetupColumn("Gain");
		ImGui::TableSetupColumn("LowNote");
		ImGui::TableSetupColumn("HighNote");
		ImGui::TableSetupColumn("LowVelocity");
		ImGui::TableSetupColumn("HighVelocity");
		ImGui::TableSetupColumn("LoopStart");
		ImGui::TableSetupColumn("LoopEnd");
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableHeadersRow();
		for (auto& zone : instrument->zones) {
			string name = getFileNameWithoutExt(AssetInfo::getPath(zone.audioData));
			ImGui::TableNextColumn();
			ImGui::Text(name.c_str());
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.audioData->getRawBufferSize());
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.audioData->getFrequency());
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.rootNote);
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.fineTune);
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.gain);
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.lowNote);
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.highNote);
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.lowVelocity);
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.highVelocity);
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.loopStart);
			ImGui::TableNextColumn();
			ImGui::Text("%d", (int)zone.loopEnd);
		}
		Asset* selectedAsset = EditorManager::getSelectedAsset();
		if (selectedAsset && &selectedAsset->assetInfo == &AudioDataAssetInfo::getInstance()) {
			ImGui::TableNextColumn();
			if (ImGui::Button(("Add " + selectedAsset->name).c_str())) {
				instrument->addAudioData(*(AudioData*)selectedAsset->load());
				instrument->build();
			}
			if (ImGui::Button("Add From Folder")) {
				FolderDlgDesc desc;
				desc.initDir = "Content";
				desc.title = "Choose Wav Folder";
				if (openFolderDlg(desc)) {
					for (auto entry : filesystem::directory_iterator(filesystem::u8path(desc.folderPath))) {
						if (!entry.is_regular_file())
							continue;
						string path = getGoodRelativePath(entry.path().generic_u8string());
						Asset* asset = AssetInfo::getAssetByPath(path);
						if (asset && &asset->assetInfo == &AudioDataAssetInfo::getInstance()) {
							instrument->addAudioData(*(AudioData*)asset->load());
						}
					}
					instrument->build();
				}
			}
		}

		ImGui::EndTable();
	}
}

struct ImGuiPianoStyles
{
	ImU32 Colors[5]{
		IM_COL32(255, 255, 255, 255),	// light note
		IM_COL32(0, 0, 0, 255),			// dark note
		IM_COL32(160, 189, 248, 255),	// active light note
		IM_COL32(42, 72, 131, 255),	// active dark note
		IM_COL32(75, 75, 75, 255),		// background
	};
	float NoteDarkHeight = 2.0f / 3.0f; // dark note scale h
	float NoteDarkWidth = 2.0f / 3.0f;	// dark note scale w
};

void MidiInstrumentEditor::onInstrumentGUI(EditorInfo& info)
{
	static int NoteIsDark[12] = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
	static int NoteLightNumber[12] = { 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6, 7 };
	static float NoteDarkOffset[12] = { 0.0f,  -2.0f / 3.0f, 0.0f, -1.0f / 3.0f, 0.0f, 0.0f, -2.0f / 3.0f, 0.0f, -0.5f, 0.0f, -1.0f / 3.0f, 0.0f };

	ImGuiID IDName = ImGui::GetID("PianoKeyBoard");

	// fix range dark keys
	if (NoteIsDark[beginOctaveNote % 12] > 0) beginOctaveNote++;
	if (NoteIsDark[endOctaveNote % 12] > 0) endOctaveNote--;

	// bad range
	if (!IDName || beginOctaveNote < 0 || endOctaveNote < 0 || endOctaveNote <= beginOctaveNote) return;

	// style
	static ImGuiPianoStyles ColorsBase;
	ImGuiPianoStyles* Style = &ColorsBase;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	const ImGuiID id = window->GetID(IDName);

	ImDrawList* draw_list = window->DrawList;

	ImVec2 Pos = window->DC.CursorPos;
	ImVec2 MousePos = ImGui::GetIO().MousePos;

	// sizes
	int CountNotesAllign7 = (endOctaveNote / 12 - beginOctaveNote / 12) * 7 + NoteLightNumber[endOctaveNote % 12] - (NoteLightNumber[beginOctaveNote % 12] - 1);

	ImVec2 Size = { ImGui::GetWindowContentRegionWidth(), 50 };

	float NoteHeight = Size.y;
	float NoteWidth = Size.x / (float)CountNotesAllign7;

	float NoteHeight2 = NoteHeight * Style->NoteDarkHeight;
	float NoteWidth2 = NoteWidth * Style->NoteDarkWidth;

	// minimal size draw
	if (NoteHeight < 5.0 || NoteWidth < 3.0) return;

	// minimal size using mouse
	bool isMouseInput = (NoteHeight >= 10.0 && NoteWidth >= 5.0);

	// item
	const ImRect bb(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y);
	ImGui::ItemSize(Size, 0);
	if (!ImGui::ItemAdd(bb, id)) return;

	// item input
	bool held = false;
	if (isMouseInput) {
		ImGui::ButtonBehavior(bb, id, nullptr, &held, 0);
	}

	MidiDevice& device = MidiDevice::defaultDevice();
	MidiState& midiState = device.state;

	int		NoteMouseCollision = -1;
	float	NoteMouseVel = 0.0f;

	float OffsetX = bb.Min.x;
	float OffsetY = bb.Min.y;
	float OffsetY2 = OffsetY + NoteHeight;
	for (int RealNum = beginOctaveNote; RealNum <= endOctaveNote; RealNum++) {
		int Octave = RealNum / 12;
		int i = RealNum % 12;

		if (NoteIsDark[i] > 0) continue;

		ImRect NoteRect(
			round(OffsetX),
			OffsetY,
			round(OffsetX + NoteWidth),
			OffsetY2
		);

		if (held && NoteRect.Contains(MousePos)) {
			NoteMouseCollision = RealNum;
			NoteMouseVel = (MousePos.y - NoteRect.Min.y) / NoteHeight;
		}

		MidiNoteState& noteState = midiState.channelStates[keyboardChannel].noteStates[RealNum];

		draw_list->AddRectFilled(NoteRect.Min, NoteRect.Max, Style->Colors[noteState.isOn ? 2 : 0], 0.0f);

		draw_list->AddRect(NoteRect.Min, NoteRect.Max, Style->Colors[4], 0.0f);

		OffsetX += NoteWidth;
	}

	// draw dark notes
	OffsetX = bb.Min.x;
	OffsetY = bb.Min.y;
	OffsetY2 = OffsetY + NoteHeight2;
	for (int RealNum = beginOctaveNote; RealNum <= endOctaveNote; RealNum++) {
		int Octave = RealNum / 12;
		int i = RealNum % 12;

		if (NoteIsDark[i] == 0) {
			OffsetX += NoteWidth;
			continue;
		}

		float OffsetDark = NoteDarkOffset[i] * NoteWidth2;
		ImRect NoteRect(
			round(OffsetX + OffsetDark),
			OffsetY,
			round(OffsetX + NoteWidth2 + OffsetDark),
			OffsetY2
		);

		if (held && NoteRect.Contains(MousePos)) {
			NoteMouseCollision = RealNum;
			NoteMouseVel = (MousePos.y - NoteRect.Min.y) / NoteHeight2;
		}

		MidiNoteState& noteState = midiState.channelStates[keyboardChannel].noteStates[RealNum];

		draw_list->AddRectFilled(NoteRect.Min, NoteRect.Max, Style->Colors[noteState.isOn ? 3 : 1], 0.0f);

		draw_list->AddRect(NoteRect.Min, NoteRect.Max, Style->Colors[4], 0.0f);
	}

	// mouse input
	if (prevMouseNote != NoteMouseCollision) {
		pianoCallback(0, prevMouseNote, 0.0f);
		prevMouseNote = 128;

		if (held && NoteMouseCollision >= 0) {
			pianoCallback(1, NoteMouseCollision, NoteMouseVel);
			prevMouseNote = NoteMouseCollision;
		}
	}

	// key input - octave control
	if (ImGui::IsKeyPressed('=') && keyboardOctave < 8) {
		keyboardOctave++;
	}
	else if (ImGui::IsKeyPressed('-') && keyboardOctave > 0) {
		keyboardOctave--;
	}
}

void MidiInstrumentEditor::pianoCallback(int Msg, uint8_t Key, float Vel)
{
	if ((Key > 108) || (Key < 21)) return; // midi max keys
	MidiState& midiState = MidiDevice::defaultDevice().state;
	MidiNoteState& noteState = midiState.channelStates[keyboardChannel].noteStates[Key];
	if (Msg == 1) {
		// if this key is not in the buffer
		if (!noteState.isOn) {
			midiState.emitNoteMessage(0, Key, true, Vel * 0x7f);
		}
	}
	if (Msg == 0) {
		// if the key is in the buffer
		if (noteState.isOn) {
			midiState.emitNoteMessage(0, Key, false, Vel * 0x7f);
		}
	}
}

void MidiInstrumentEditor::onGUI(EditorInfo& info)
{
	if (instrument == NULL)
		return;
	BaseEditor::onInstanceGUI(info);
	onMidiDeviceGUI(info);
	onConfigGUI(info);
	onInstrumentGUI(info);
}
