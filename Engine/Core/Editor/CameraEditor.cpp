#include "CameraEditor.h"
#include "../Engine.h"
#include "../GUI/GUIUtility.h"
#include "../SkeletonMeshActor.h"
#include "../GUI/GizmoUltility.h"

RegistEditor(Camera);

Texture2D* CameraEditor::icon = NULL;

void CameraEditor::setInspectedObject(void* object)
{
	camera = dynamic_cast<Camera*>((Object*)object);
	TransformEditor::setInspectedObject(camera);
}

void CameraEditor::onPersistentGizmo(GizmoInfo& info)
{
	Texture2D* icon = getIcon();

	Vector3f pos = camera->getFinalWorldPosition();

	if (icon != NULL) {
		info.gizmo->drawIcon(*icon, pos, getFitIconSize(info));
		if (info.gizmo->pickIcon(pos, Vector2f::Zero(),
			{ (float)icon->getWidth(), (float)icon->getHeight() }))
			EditorManager::selectObject(camera);
	}
}

void CameraEditor::onHandleGizmo(GizmoInfo& info)
{
	TransformEditor::onHandleGizmo(info);
	drawFrustum(*info.gizmo, camera->cameraRender.cameraData, Color(0.0f, 1.0f, 0.0f));
}

void CameraEditor::onCameraGUI(EditorInfo& info)
{
	World& world = *Engine::getCurrentWorld();
	if (&world.getCurrentCamera() != camera)
		if (ImGui::Button("SetCamera", { -1, 36 }))
			world.switchCamera(*camera);
	ImGui::Checkbox("EditorControl", &gizmoCameraInfo.canControlledByEditor);
	ImGui::DragFloat("EditorSpeed", &gizmoCameraInfo.speedInEditor, 0.01f, 0, 128);
	Enum<CameraRenderFlags> flags = camera->cameraRender.getCameraRenderFlags();
	bool debugDraw = flags.has(CameraRender_DebugDraw);
	ImGui::Checkbox("DebugDraw", &debugDraw);
	camera->cameraRender.setCameraRenderFlags(debugDraw ? (flags.enumValue | CameraRender_DebugDraw) : (flags.enumValue & ~CameraRender_DebugDraw));
	float fov = camera->fov;
	if (ImGui::DragFloat("FOV", &fov, 0.1, 0, 180)) {
		camera->fov = fov;
	}
	float zNear = camera->zNear;
	if (ImGui::DragFloat("zNear", &zNear, 0.1, 0, 1000.0f)) {
		camera->zNear = zNear;
	}
	float zFar = camera->zFar;
	if (ImGui::DragFloat("zFar", &zFar, 0.1, 0, 100000000.0f)) {
		camera->zFar = zFar;
	}
	float distance = camera->distance;
	if (ImGui::DragFloat("Distance", &distance, 0.1)) {
		camera->distance = distance;
	}
	Color clearColor = camera->clearColor;
	if (ImGui::ColorEdit4("ClearColor", (float*)&clearColor))
		camera->clearColor = clearColor;
	int massLevel = camera->cameraRender.getRenderTarget().getMultisampleLevel() / 4;
	if (ImGui::Combo("MSAA", &massLevel, "None\0""4x\0""8x\0""16x\0")) {
		camera->cameraRender.getRenderTarget().setMultisampleLevel(massLevel * 4);
	}
}

void CameraEditor::onAnimation(EditorInfo& info)
{
	if (ImGui::Button("Add Select Camera Animation", { -1, 40 })) {
		Asset* assignAsset = EditorManager::getSelectedAsset();
		if (assignAsset != NULL && assignAsset->assetInfo.type == "AnimationClipData") {
			camera->setAnimationClip(*(AnimationClipData*)assignAsset->load());
		}
	}
	static string inputName;
	static SkeletonMeshActor* chr = NULL;
	Object* chrObj = chr;
	ImGui::ObjectCombo("Target", chrObj, info.world->getObject(), inputName);
	chr = dynamic_cast<SkeletonMeshActor*>(chrObj);

	static int activeAnimIndex = 0;
	static int activeAudioIndex = 0;
	if (chr != NULL) {
		string animStr;
		vector<unsigned int> animIndexes;
		if (chr->animationClipList.empty())
			activeAnimIndex = -1;
		else for (auto b = chr->animationClipList.begin(), e = chr->animationClipList.end(); b != e; b++) {
			animStr += b->first + '\0';
			animIndexes.push_back(b->second);
		}
		ImGui::Combo("Anim", &activeAnimIndex, animStr.c_str());

#ifdef AUDIO_USE_OPENAL
		string audioStr;
		if (chr->audioSources.empty())
			activeAudioIndex = -1;
		else for (auto b = chr->audioSources.begin(), e = chr->audioSources.end(); b != e; b++) {
			audioStr += (*b)->audioData->name + '\0';
		}
		ImGui::Combo("Audio", &activeAudioIndex, audioStr.c_str());
#endif // AUDIO_USE_OPENAL

		if (camera->animationClip.animationClipData != NULL && activeAnimIndex >= 0) {
			if (camera->animationClip.playing()) {
				if (ImGui::Button("Pause")) {
					camera->animationClip.pause();
					chr->activeAnimationClip(animIndexes[activeAnimIndex]);
					chr->animationClip->pause();
#ifdef AUDIO_USE_OPENAL
					if (activeAudioIndex >= 0)
						chr->audioSources[activeAudioIndex]->pause();
#endif // AUDIO_USE_OPENAL
				}
				if (ImGui::Button("Stop")) {
					camera->animationClip.stop();
					chr->activeAnimationClip(animIndexes[activeAnimIndex]);
					chr->animationClip->stop();
#ifdef AUDIO_USE_OPENAL
					if (activeAudioIndex >= 0)
						chr->audioSources[activeAudioIndex]->stop();
#endif // AUDIO_USE_OPENAL
				}
			}
			else if (ImGui::Button("Play")) {
				camera->animationClip.play();
				chr->activeAnimationClip(animIndexes[activeAnimIndex]);
				chr->animationClip->play();
#ifdef AUDIO_USE_OPENAL
				if (activeAudioIndex >= 0)
					chr->audioSources[activeAudioIndex]->play();
#endif // AUDIO_USE_OPENAL
			}
		}
	}
}

void CameraEditor::onPostprocess(EditorInfo& info)
{
	if (camera->cameraRender.graph == NULL || camera->cameraRender.graph->passes.empty()) {
		if (ImGui::Button("AddDefaultPass")) {
			camera->cameraRender.createDefaultPostProcessGraph();
		}
		return;
	}
	PostProcessGraph& graph = *camera->cameraRender.graph;
	int i = 0;
	for (auto b = graph.passes.begin(), e = graph.passes.end(); b != e; b++, i++) {
		ImGui::PushID(i);
		bool enable = (*b)->getEnable();
		if (ImGui::Checkbox("##Enable", &enable)) {
			(*b)->setEnable(enable);
		}
		ImGui::SameLine();
		if (ImGui::CollapsingHeader((*b)->getName().c_str())) {
			ImGui::Indent(20);
			(*b)->onGUI(info);
			Editor* editor = EditorManager::getEditor("Material", (*b)->getMaterial());
			if (editor != NULL) {
				editor->onGUI(info);
			}
			ImGui::Unindent(20);
		}
		ImGui::PopID();
	}
}

void CameraEditor::onDetailGUI(EditorInfo& info)
{
	TransformEditor::onDetailGUI(info);
	if (ImGui::CollapsingHeader("Camera"))
		onCameraGUI(info);
	if (ImGui::CollapsingHeader("Animation"))
		onAnimation(info);
	if (ImGui::CollapsingHeader("Postprocess"))
		onPostprocess(info);
}

GizmoCameraInfo CameraEditor::getGizmoCameraInfo(Camera* camera)
{
	const GizmoCameraInfo defaultInfo = { false, 0 };
	if (camera == NULL)
		return defaultInfo;
	CameraEditor* editor = dynamic_cast<CameraEditor*>(EditorManager::getEditor(*camera));
	if (editor == NULL)
		return defaultInfo;
	return editor->gizmoCameraInfo;
}

Texture2D* CameraEditor::getIcon()
{
	if (icon == NULL)
		icon = getAssetByPath<Texture2D>("Engine/Icons/Camera_Icon.png");
	return icon;
}
