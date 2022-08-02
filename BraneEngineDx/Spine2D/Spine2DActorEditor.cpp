#include "Spine2DActorEditor.h"
#include "../Core/GUI/TextureViewer.h"

RegistEditor(Spine2DActor);

void Spine2DActorEditor::setInspectedObject(void* object)
{
    spine2DActor = dynamic_cast<Spine2DActor*>((Object*)object);
    ActorEditor::setInspectedObject(spine2DActor);
}

Texture* Spine2DActorEditor::getPreviewTexture(const Vector2f& desiredSize)
{
	if (spine2DActor == NULL)
		return NULL;
	if (spine2DActor->spine2DRender.get3DMode())
		return NULL;
	spine2DActor->spine2DRender.setRefSize(desiredSize.x());
	return spine2DActor->spine2DRender.getOutputTexture();
}

void Spine2DActorEditor::onActorGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("Skins")) {
		ImGui::Indent(10);
		const auto model = spine2DActor->getModel();
		const int count = model->getSkinCount();
		for (int i = 0; i < count; i++) {
			ImGui::PushID(i);
			if (ImGui::Button(model->getSkinName(i).c_str(), { -1, 36 })) {
				spine2DActor->setSkin(i);
			}
			ImGui::PopID();
		}
		ImGui::Unindent(10);
	}

	if (ImGui::CollapsingHeader("Slots")) {
		ImGui::Indent(10);
		const auto skeleton = spine2DActor->getSkeleton();
		const int count = skeleton->getSlots().size();
		for (int i = 0; i < count; i++) {
			spine::Slot* slot = skeleton->getDrawOrder()[i];
			ImGui::BulletText("%s", slot->getData().getName().buffer());
			spine::Attachment* attachment = slot->getAttachment();
			ImGui::Indent(10);
			ImGui::Text("%s", attachment == NULL ? "null" : attachment->getRTTI().getClassName());
			ImGui::Unindent(10);
		}
		ImGui::Unindent(10);
	}

	if (ImGui::CollapsingHeader("Animations")) {
		ImGui::Indent(10);
		/*if (ImGui::BeginCombo("ActivedTrack", ("Track " + to_string(activeTrackIndex)).c_str())) {
			int count = spine2DActor->getAnimationTrackCount();
			for (int i = 0; i < count; i++) {
				bool selected = activeTrackIndex == i;
				if (ImGui::Selectable(("Track " + to_string(i)).c_str(), &selected)) {
					activeTrackIndex = i;
				}
			}
			ImGui::EndCombo();
		}*/
		if (ImGui::InputInt("TrackIndex", &activeTrackIndex)) {
			activeTrackIndex = max(activeTrackIndex, 0);
		}
		ImGui::Checkbox("Loop", &loop);
		int count = spine2DActor->getModel()->getAnimationCount();
		for (int i = 0; i < count; i++) {
			ImGui::PushID(i);
			Spine2DAnimation animation = spine2DActor->getModel()->getAnimation(i);
			if (ImGui::Button(animation.getName().c_str(), { -1, 36 })) {
				spine2DActor->playAnimation(activeTrackIndex, animation, loop);
			}
			ImGui::PopID();
		}
		ImGui::Unindent(10);
	}
}

void Spine2DActorEditor::onRenderersGUI(EditorInfo& info)
{
	if (spine2DActor->getModel() == NULL) {
		ImGui::Text("No Spine2D Model");
	}
	else {
		string modelPath = MeshAssetInfo::getPath(spine2DActor->getModel());
		ImGui::Text(modelPath.c_str());
	}

	Asset* assignAsset = EditorManager::getSelectedAsset();
	if (assignAsset != NULL && assignAsset->assetInfo.type == "Spine2DModel") {
		if (ImGui::Button("Set Selected Spine2DModel", { -1, 40 })) {
			spine2DActor->setModel((Spine2DModel*)assignAsset->load());
		}
	}

	bool is3DMode = spine2DActor->spine2DRender.get3DMode();
	if (ImGui::Checkbox("Is3DMode", &is3DMode))
		spine2DActor->spine2DRender.set3DMode(is3DMode);

	Texture2D* outputTexture = spine2DActor->spine2DRender.getOutputTexture();

	if (outputTexture != NULL) {
		float width = ImGui::GetWindowContentRegionWidth();
		unsigned long long id = outputTexture->getTextureID();
		float height = width * outputTexture->getHeight() / outputTexture->getWidth();
		if (ImGui::ImageButton((ImTextureID)id, { width, height })) {
			TextureViewer::showTexture(*info.gui, *outputTexture);
		}
	}
}
