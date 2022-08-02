#include "ActorEditor.h"
#include "../MeshRender.h"
#include "../SkeletonMeshRender.h"

RegistEditor(Actor);

void ActorEditor::setInspectedObject(void* object)
{
	actor = dynamic_cast<Actor*>((Object*)object);
	TransformEditor::setInspectedObject(actor);
}

void ActorEditor::onAudioGUI(EditorInfo& info)
{
#ifdef AUDIO_USE_OPENAL
	ImGui::Text("AudioSource: %d", actor->audioSources.size());
	Asset* assignAsset = EditorManager::getSelectedAsset();
	if (assignAsset != NULL && assignAsset->assetInfo.type == "AudioData") {
		if (ImGui::Button("Add Selected Audio", { -1, 36 })) {
			AudioData* auData = (AudioData*)assignAsset->load();
			if (auData != NULL)
				actor->addAudioSource(*auData);
		}
	}
	ImGui::Separator();
	for (int i = 0; i < actor->audioSources.size(); i++) {
		ImGui::Text(actor->audioSources[i]->audioData->name.c_str());
		ImGui::SameLine();
		if (actor->audioSources[i]->getState() == AudioSource::Playing) {
			if (ImGui::Button(("Pause##Pause" + to_string(i)).c_str()))
				actor->audioSources[i]->pause();
			ImGui::SameLine();
			if (ImGui::Button(("Stop##Stop" + to_string(i)).c_str()))
				actor->audioSources[i]->stop();
		}
		else if (ImGui::Button(("Play##Play" + to_string(i)).c_str()))
			actor->audioSources[i]->play();
	}
#endif // AUDIO_USE_OPENAL
}

void ActorEditor::onActorGUI(EditorInfo& info)
{
	if (actor->isHidden()) {
		if (ImGui::Button("Show Actor"))
			actor->setHidden(false);
	}
	else
		if (ImGui::Button("Hide Actor"))
			actor->setHidden(true);
	if (ImGui::Button("Collision")) {
		ImGui::OpenPopup("ActorCollision");
	}
}

void ActorEditor::onRenderersGUI(EditorInfo& info)
{
	vector<Render*> renders;
	int count = actor->getRenders(renders);
	for (int i = 0; i < count; i++) {
		Render* render = renders[i];
		if (ImGui::CollapsingHeader(("Render " + to_string(i)).c_str())) {
			MeshRender* mr = dynamic_cast<MeshRender*>(render);
			if (mr) {
				Editor* editor = EditorManager::getEditor("MeshRender", mr);
				editor->onGUI(info);
			}
			SkeletonMeshRender* smr = dynamic_cast<SkeletonMeshRender*>(render);
			if (smr) {
				Editor* editor = EditorManager::getEditor("SkeletonMeshRender", smr);
				editor->onGUI(info);
			}
		}
	}
}

void ActorEditor::onDetailGUI(EditorInfo& info)
{
	TransformEditor::onDetailGUI(info);
	if (ImGui::CollapsingHeader("Audio"))
		onAudioGUI(info);
	if (ImGui::CollapsingHeader("Actor"))
		onActorGUI(info);
	if (ImGui::CollapsingHeader("Render"))
		onRenderersGUI(info);
}
