#include "AudioPlayableEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(AudioPlayable);

void AudioPlayableEditor::setInspectedObject(void* object)
{
	audioPlayable = dynamic_cast<AudioPlayable*>((TimelinePlayable*)object);
	TimelinePlayableEditor::setInspectedObject(audioPlayable);
}

void AudioPlayableEditor::onPlayableGUI(EditorInfo& info)
{
	AudioData* data = audioPlayable->getAudio();
	Asset* asset = AudioDataAssetInfo::assetInfo.getAsset(data);
	Asset* _asset = asset;
	ImGui::AssetCombo("Audio", _asset, "AudioData");
	if (_asset != asset) {
		if (_asset == NULL) {
			audioPlayable->setAudio(NULL);
		}
		else {
			audioPlayable->setAudio((AudioData*)_asset->load());
		}
	}
}
