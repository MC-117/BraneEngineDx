#include "ESCMenu.h"
#include "../Engine.h"
#include "../Console.h"
#include "EditorWindow.h"

ESCMenu::ESCMenu(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
	backgroundColor = { 0, 0, 0, 0 };
	showCloseButton = false;
	setStyle(ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_UnsavedDocument |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
}

ESCMenu::~ESCMenu()
{
}

void ESCMenu::onPreAction(GUIRenderInfo& info)
{
	World* world = Engine::getCurrentWorld();
	if (world == NULL)
		return;
	Input& input = world->input;
	if (input.getKeyPress(VK_ESCAPE))
		show = !show;
	if (show)
		input.setCursorHidden(false);
	else {
		if (!info.gui.isShowUIControl("Editor"))
			input.setCursorHidden(true);
	}
}

void ESCMenu::onRenderWindow(GUIRenderInfo& info)
{
	ImVec2 ws = { 200, 142 };
	ImGui::SetWindowSize(ws, ImGuiCond_Always);
	ImGui::SetWindowPos({ (info.viewSize.x - ws.x) * 0.5f, (info.viewSize.y - ws.y) * 0.5f }, ImGuiCond_Always);
	ImVec2 size = { ImGui::GetWindowContentRegionWidth(), 40 };

	World* world = Engine::getCurrentWorld();
	if (world == NULL)
		return;

	if (ImGui::Button("Play", size)) {
		bool ok = true;
		Object* obj = (Object*)Brane::find(typeid(Object).hash_code(), "Miku");
		SkeletonMeshActor* miku = dynamic_cast<SkeletonMeshActor*>(obj);
		if (miku == NULL) {
			ok = false;
			Console::error("Not found SkeletonMeshActor(Miku)");
		}
		AudioData* audio = getAssetByPath<AudioData>("Content/Scene/MagicMiku/Weekender Girl.wav");
		if (audio == NULL) {
			ok = false;
			Console::error("Not found Content/Scene/MagicMiku/Weekender Girl.wav");
		}
		AnimationClipData* anim = getAssetByPath<AnimationClipData>("Content/Scene/MagicMiku/Weekender_Girl_MagicMiku.charanim");
		if (anim == NULL) {
			ok = false;
			Console::error("Not found Content/Scene/MagicMiku/Weekender_Girl_MagicMiku.charanim");
		}
		AnimationClipData* camAnim = getAssetByPath<AnimationClipData>("Content/Scene/MagicMiku/Weekender_Girl_camera.camanim");
		if (camAnim == NULL) {
			ok = false;
			Console::error("Content/Scene/MagicMiku/Weekender_Girl_camera.camanim");
		}
		if (ok) {
			bool has = false;
			for (auto b = miku->animationClips.begin(), e = miku->animationClips.end(); b != e; b++) {
				(*b)->stop();
				AnimationClip* clip = dynamic_cast<AnimationClip*>(*b);
				if (clip != NULL)
					if (clip->animationClipData == anim)
						has = true;
			}
			if (!has)
				miku->addAnimationClip(*anim);
			has = false;
			AudioSource* audioSource = NULL;
			for (auto b = miku->audioSources.begin(), e = miku->audioSources.end(); b != e; b++) {
				(*b)->stop();
				if ((*b)->audioData == audio) {
					audioSource = *b;
					has = true;
				}
			}
			if (!has)
				audioSource = miku->addAudioSource(*audio);
			Camera& cam = world->getCurrentCamera();
			if (cam.animationClip.animationClipData != camAnim)
				cam.setAnimationClip(*camAnim);
			cam.animationClip.stop();

			miku->activeAnimationClip(anim->name);
			miku->animationClip->play();
			cam.animationClip.play();
			audioSource->play();
		}
	}
	if (ImGui::Button("ShowEditor", size)) {
		UIControl* uc = info.gui.getUIControl("Editor");
		bool b = false;
		if (uc != NULL) {
			EditorWindow* win = dynamic_cast<EditorWindow*>(uc);
			if (win != NULL) {
				win->show = true;
			}
			else
				b = true;
		}
		else
			b = true;
		if (b) {
			EditorWindow* win = new EditorWindow(*world, Material::nullMaterial, "Editor", true);
			(*world) += win;
		}
	}
	if (ImGui::Button("Quit", size)) {
		world->quit();
	}
}
