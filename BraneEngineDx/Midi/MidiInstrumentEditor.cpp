#include "MidiInstrumentEditor.h"
#include "../Core/Utility/EngineUtility.h"
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
}

void MidiInstrumentEditor::onConfigGUI(EditorInfo& info)
{
	if (ImGui::BeginTable("Zones", 10, ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {
		ImGui::TableSetupColumn("Name");
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

void MidiInstrumentEditor::onInstrumentGUI(EditorInfo& info)
{
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
