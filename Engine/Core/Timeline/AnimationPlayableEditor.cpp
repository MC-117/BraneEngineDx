#include "AnimationPlayableEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(AnimationPlayable);

void AnimationPlayableEditor::setInspectedObject(void* object)
{
	animPlayable = dynamic_cast<AnimationPlayable*>((TimelinePlayable*)object);
	TimelinePlayableEditor::setInspectedObject(animPlayable);
}

void AnimationPlayableEditor::onPlayableGUI(EditorInfo& info)
{
	AnimationClipData* data = animPlayable->getAnimation();
	Asset* asset = AnimationClipDataAssetInfo::assetInfo.getAsset(data);
	Asset* _asset = asset;
	ImGui::AssetCombo("Animation", _asset, "AnimationClipData");
	if (_asset != asset) {
		if (_asset == NULL) {
			animPlayable->setAnimation(NULL);
		}
		else {
			animPlayable->setAnimation((AnimationClipData*)_asset->load());
		}
	}
	const char* BlendModeNames[] = { "Replace", "Additive" };
	AnimationBlendMode mode = animPlayable->getAnimationBlendMode();
	if (ImGui::BeginCombo("BlendMode", BlendModeNames[(int)mode])) {
		if (ImGui::Selectable(BlendModeNames[(int)AnimationBlendMode::Replace])) {
			animPlayable->setAnimationBlendMode(AnimationBlendMode::Replace);
		}
		if (ImGui::Selectable(BlendModeNames[(int)AnimationBlendMode::Additive])) {
			animPlayable->setAnimationBlendMode(AnimationBlendMode::Additive);
		}
		ImGui::EndCombo();
	}
}
