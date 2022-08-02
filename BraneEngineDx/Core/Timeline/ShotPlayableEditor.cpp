#include "ShotPlayableEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(ShotPlayable);

void ShotPlayableEditor::setInspectedObject(void* object)
{
	shotPlayable = dynamic_cast<ShotPlayable*>((TimelinePlayable*)object);
	TimelinePlayableEditor::setInspectedObject(shotPlayable);
}

void ShotPlayableEditor::onPlayableGUI(EditorInfo& info)
{
	Camera* camera = shotPlayable->getCamera();
	if (ImGui::ObjectCombo("Camera", (Object*&)camera, info.world->getObject(), "Camera"))
		shotPlayable->setCamera(camera);

	AnimationClipData* data = shotPlayable->getAnimation();
	Asset* asset = AnimationClipDataAssetInfo::assetInfo.getAsset(data);
	if (ImGui::AssetCombo("Animation", asset, "AnimationClipData")) {
		if (asset == NULL) {
			shotPlayable->setAnimation(NULL);
		}
		else {
			shotPlayable->setAnimation((AnimationClipData*)asset->load());
		}
	}
}
