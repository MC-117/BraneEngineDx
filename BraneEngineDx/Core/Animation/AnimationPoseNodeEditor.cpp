#include "AnimationPoseNodeEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(AnimationBaseNode);

void AnimationBaseNodeEditor::setInspectedObject(void* object)
{
	animationBaseNode = dynamic_cast<AnimationBaseNode*>((Base*)object);
	GraphNodeEditor::setInspectedObject(animationBaseNode);
}

void AnimationBaseNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (node == NULL)
		return;

	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;

	ne::NodeId nid = node->getInstanceID();

	graphInfo.builder->Begin(nid);

	graphInfo.builder->Header((ImVec4&)node->getNodeColor());
	ImGui::Spring(0);
	ImGui::TextUnformatted(node->getDisplayName().c_str());
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	graphInfo.builder->EndHeader();

	for (int i = 0; i < node->getInputCount(); i++) {
		GraphPin* pin = node->getInput(i);
		if (!pin)
			continue;
		GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
			EditorManager::getEditor(*pin));
		if (pinEditor)
			pinEditor->onGraphGUI(info, graphInfo);
	}

	graphInfo.builder->Middle();

	ImGui::TextUnformatted(animationBaseNode->getAnimationName().c_str());

	float time = animationBaseNode->getNormalizedTime();
	ImGui::ProgressBar(time, { 100, 16 });

	for (int i = 0; i < node->getOutputCount(); i++) {
		GraphPin* pin = node->getOutput(i);
		if (!pin)
			continue;
		GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
			EditorManager::getEditor(*pin));
		if (pinEditor)
			pinEditor->onGraphGUI(info, graphInfo);
	}

	graphInfo.builder->End();
}

void AnimationBaseNodeEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	ImGui::BeginHorizontal("##AnimationBaseNodeH");

	if (animationBaseNode->getPlaying()) {
		if (ImGui::Button(ICON_FA_PAUSE, { 25, 25 })) {
			animationBaseNode->setPlaying(false);
		}
	}
	else {
		if (ImGui::Button(ICON_FA_PLAY, { 25, 25 })) {
			animationBaseNode->setPlaying(true);
		}
	}
	bool loop = animationBaseNode->getLoop();
	if (ImGui::Checkbox("Loop", &loop))
		animationBaseNode->setLoop(loop);
	bool autoPlay = animationBaseNode->getAutoPlay();
	if (ImGui::Checkbox("AutoPlay", &autoPlay))
		animationBaseNode->setAutoPlay(autoPlay);
	ImGui::EndHorizontal();
}

RegistEditor(AnimationClipNode);

void AnimationClipNodeEditor::setInspectedObject(void* object)
{
	animationClipNode = dynamic_cast<AnimationClipNode*>((Base*)object);
	AnimationBaseNodeEditor::setInspectedObject(animationClipNode);
}

void AnimationClipNodeEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	AnimationBaseNodeEditor::onContextMenuGUI(info, graphInfo);
	Asset* asset = AnimationClipDataAssetInfo::assetInfo.getAsset(animationClipNode->getAnimation());
	ImGui::SetNextItemWidth(120);
	if (ImGui::AssetCombo("Animation", asset, "AnimationClipData"))
		animationClipNode->setAnimation((AnimationClipData*)asset->load());
}
