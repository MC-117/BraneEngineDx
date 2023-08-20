#include "ESCMenu.h"
#include "../Engine.h"
#include "../Console.h"
#include "EditorWindow.h"
#include "../SkeletonMeshActor.h"
#include "../WUI/WUIMainWindow.h"

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
	Input& input = Engine::input;
	if (input.getKeyPress(VK_ESCAPE))
		show = !show;
	if (show)
		input.setCursorHidden(false);
	else {
		if (!info.gui.isShowUIControl("Editor"))
			input.setCursorHidden(true);
	}
}

void TextCenter(std::string text) {
	float font_size = ImGui::GetFontSize() * text.size() / 2;
	ImGui::SameLine(
		ImGui::GetWindowSize().x / 2 -
		font_size + (font_size / 2)
	);
	ImGui::Text(text.c_str());
}

void ESCMenu::onRenderWindow(GUIRenderInfo& info)
{
	ImVec2 ws = { 200, 252 };
	ImVec2 pos = ImGui::GetMainViewport()->WorkPos;
	ImGui::SetWindowSize(ws, ImGuiCond_Always);
	ImGui::SetWindowPos({ pos.x + (info.viewSize.x - ws.x) * 0.5f, pos.y + (info.viewSize.y - ws.y) * 0.5f }, ImGuiCond_Always);
	ImVec2 size = { ImGui::GetWindowContentRegionWidth(), 40 };

	World* world = Engine::getCurrentWorld();
	if (world == NULL)
		return;

	TextCenter("Menu");
	if (ImGui::Button("Play", size)) {
		bool ok = true;
		Object* obj = world->findChild("TDA");
		SkeletonMeshActor* miku = dynamic_cast<SkeletonMeshActor*>(obj);
		if (miku == NULL) {
			ok = false;
			Console::error("Not found SkeletonMeshActor(TDA)");
		}
#ifdef AUDIO_USE_OPENAL
		AudioData* audio = getAssetByPath<AudioData>("Content/MMD/Merankorikku.wav");
		if (audio == NULL) {
			ok = false;
			Console::error("Not found Content/MMD/Merankorikku.wav");
		}
#endif // AUDIO_USE_OPENAL
		AnimationClipData* anim = getAssetByPath<AnimationClipData>("Content/MMD/TDA/Merankorikku_TDA.charanim");
		if (anim == NULL) {
			ok = false;
			Console::error("Not found Content/MMD/TDA/Merankorikku_TDA.charanim");
		}
		AnimationClipData* camAnim = getAssetByPath<AnimationClipData>("Content/MMD/Merankorikku_Cam.camanim");
		if (camAnim == NULL) {
			ok = false;
			Console::error("Not found Content/MMD/Merankorikku_Cam.camanim");
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
#ifdef AUDIO_USE_OPENAL
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
#endif // AUDIO_USE_OPENAL
			Camera& cam = world->getCurrentCamera();
			if (cam.animationClip.animationClipData != camAnim)
				cam.setAnimationClip(*camAnim);
			cam.animationClip.stop();

			miku->activeAnimationClip(anim->name);
			miku->animationClip->play();
			cam.animationClip.play();
#ifdef AUDIO_USE_OPENAL
			audioSource->play();
#endif // AUDIO_USE_OPENAL
			info.gui.hideAllUIControl();
			Engine::input.setCursorHidden(true);
		}
		/*info.gui.hideAllUIControl();
		world->input.setCursorHidden(true);*/
	}
#ifdef AUDIO_USE_OPENAL
	int mainVolume = world->audioListener.getVolume() * 100;
	if (ImGui::SliderInt("Volume", &mainVolume, 0, 100)) {
		world->audioListener.setVolume(mainVolume / 100.f);
	}
#endif // AUDIO_USE_OPENAL
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
		info.gui.gizmo.setEnableGUI(true);
	}
	if (ImGui::Button("Toggle Fullscreen", size)) {
		Engine::mainWindow.toggleFullscreen();
	}
	if (ImGui::Button("Quit", size)) {
		world->quit();
	}
}
